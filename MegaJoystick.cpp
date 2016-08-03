/*
  MegaJoystick.cpp
  
  copyright (c) 2016, Piotr Zapart
  
  based on/inspired by:  

  - Arduino Joystick library Copyright (c) 2015, Matthew Heironimus
  - Teensy Extreme Joystick library by Paul Stoffregen/PJRC.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "MegaJoystick.h"


#if defined(_USING_HID)

#define JOYSTICK_REPORT_ID 0x03
#define JOYSTICK_STATE_SIZE 64

#define HAT_BIT_UP		0
#define HAT_BIT_RIGHT	1
#define HAT_BIT_DOWN	2
#define HAT_BIT_LEFT	3

static const int8_t _hat_decoder[13] PROGMEM =
{
	8,	//OFF
	0,	//UP
	2,	//RIGHT
	1,	//UP+RIGHT
	4,	//DOWN
	8,	//OFF
	3,	//DOWN+RIGHT
	8,	//OFF
	6,	//LEFT
	7,	//UP+LEFT
	8,	//OFF
	8,	//OFF
	5	//DOWN+LEFT
};



static const uint8_t _hidReportDescriptor[] PROGMEM = 
{
	
		// Joystick
	0x05, 0x01,			      		// USAGE_PAGE (Generic Desktop)
	0x09, 0x04,			      		// USAGE (Joystick)
	0xa1, 0x01,			      		// COLLECTION (Application)
	0x85, JOYSTICK_REPORT_ID, 		//   REPORT_ID (3)

	// 128 Buttons
	0x05, 0x09,			      		// USAGE_PAGE (Button)
	0x15, 0x00,                     // Logical Minimum (0)
	0x25, 0x01,                     // Logical Maximum (1)
	0x75, 0x01,                     // Report Size (1)
	0x95, 0x80,                     // Report Count (128)
	0x19, 0x01,                     // Usage Minimum (Button #1)
	0x29, 0x80,                     // Usage Maximum (Button #128)
	0x65, 0x00,			     		// UNIT (None)
	0x81, 0x02,                     // Input (variable,absolute)
	//6 x axis + 17 x slider
	0x05, 0x01,                     // Usage Page (Generic Desktop)
	0x09, 0x01,                     // Usage (Pointer)
	0xA1, 0x00,                     // Collection ()
	0x15, 0x00,                     // Logical Minimum (0)
	0x27, 0xFF, 0xFF, 0, 0,         // Logical Maximum (65535)
	0x75, 0x10,                     // Report Size (16)
	0x95, 23,                       // Report Count (23)
	0x09, 0x30,                     // Usage (X)
	0x09, 0x31,                     // Usage (Y)
	0x09, 0x32,                     // Usage (Z)
	0x09, 0x33,                     // Usage (Rx)
	0x09, 0x34,                     // Usage (Ry)
	0x09, 0x35,                     // Usage (Rz)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x81, 0x02,                     // Input (variable,absolute)
	0xC0,                           // End Collection
	
	// 4x HAT
	0x15, 0x00,                     // Logical Minimum (0)
	0x25, 0x07,                     // Logical Maximum (7)
	0x35, 0x00,                     // Physical Minimum (0)
	0x46, 0x3B, 0x01,               // Physical Maximum (315)
	0x75, 0x04,                     // Report Size (4)
	0x95, 0x04,                     // Report Count (4)
	0x65, 0x14,                     // Unit (20)
	0x05, 0x01,                     // Usage Page (Generic Desktop)
	0x09, 0x39,                     // Usage (Hat switch)
	0x09, 0x39,                     // Usage (Hat switch)
	0x09, 0x39,                     // Usage (Hat switch)
	0x09, 0x39,                     // Usage (Hat switch)
	0x81, 0x42,                     // Input (variable,absolute,null_state)
	0xC0                            // End Collection
};
//================================================================================
MegaJoystick_::MegaJoystick_()
{
	// Setup HID report structure
	static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
	HID().AppendDescriptor(&node);
	
	uint8_t i; 
	//init axis & rotation
	for (i=AXIS_ADDR;i<SLIDER_ADDR;i++)
	{
		analog[i] = 32768;
	}
	//init sliders to 0
	for (i=SLIDER_ADDR;i<24;i++)
	{
		analog[i] = 0;
	}
	
	hatSwitch[0] = 0x0F;
	hatSwitch[1] = 0x0F;
	hatSwitch[2] = 0x0F;
	hatSwitch[3] = 0x0F;
}
//================================================================================
void MegaJoystick_::begin(bool initAutoSendState)
{
	autoSendState = initAutoSendState;
	sendState();
}
//================================================================================
void MegaJoystick_::end()
{
}

void MegaJoystick_::setAutoSend(bool state)
{
	autoSendState = state;
}
//================================================================================
void MegaJoystick_::setButton(uint8_t button, uint8_t value)
{
	if (value == 0)
	{
		releaseButton(button);
	}
	else
	{
		pressButton(button);
	}
}
//================================================================================
/*
	button array = 16 banks 8bit each
	single input = 0-127 >> 3 = 0-15 array index
	bit mask = 0x07 

*/
void MegaJoystick_::pressButton(uint8_t button)
{
	if (button>127)	return;
	bitSet(buttons[button>>3], button&0x07);
	if (autoSendState) sendState();
}
//================================================================================
void MegaJoystick_::releaseButton(uint8_t button)
{
	if (button>127)	return;
	bitClear(buttons[button>>3], button&0x07);
	if (autoSendState) sendState();
}
//================================================================================
bool MegaJoystick_::getButton(uint8_t button)
{
	return (buttons[button>>3] & (_BV(button&0x07)) ? true: false);
}
//================================================================================
//write a 16bit word of button data directly. Useful when using IO expanfers like MCP23017
void MegaJoystick_::setButtonBank16(uint8_t bank, uint16_t buttonStates)
{
	if (bank>15)	return;
	if (bank>14)			//write only the 1st 8 bits
	{
		buttons[bank] = (uint8_t) buttonStates;
	}
	else
	{
		buttons[bank] = (uint8_t) buttonStates;
		buttons[bank+1] = (uint8_t) (buttonStates>>8);
	}
	if (autoSendState) sendState();
}
//================================================================================
//write an 8bit word of button data directly. Useful when using IO expanfers like PCF8574, 
// shift registers and so on
void MegaJoystick_::setButtonBank8(uint8_t bank, uint8_t buttonStates)
{
	if (bank>15)		return;
	else				buttons[bank] = buttonStates;
	if (autoSendState) sendState();
}
//================================================================================
// --- SLIDERS ---
void MegaJoystick_::setSlider(uint8_t sliderNo, uint16_t value)
{
	if (sliderNo >16) return;
	analog[SLIDER_ADDR+16-sliderNo] = value;
}

uint16_t MegaJoystick_::getSlider(uint8_t sliderNo)
{	
	return analog[SLIDER_ADDR+16-sliderNo];
}
//================================================================================
// --- X AXIS ---
void MegaJoystick_::setXAxis(uint16_t value)
{
	analog[X_ADDR] = value;
	if (autoSendState) sendState();
}
//================================================================================
uint16_t MegaJoystick_::getXAxis(void)
{
	return analog[X_ADDR];
}
//================================================================================
void MegaJoystick_::setYAxis(uint16_t value)
{
	analog[Y_ADDR] = value;
	if (autoSendState) sendState();
}
uint16_t MegaJoystick_::getYAxis(void)
{
	return analog[Y_ADDR];
}
//================================================================================
void MegaJoystick_::setZAxis(uint16_t value)
{
	analog[Z_ADDR] = value;
	if (autoSendState) sendState();
}
uint16_t MegaJoystick_::getZAxis(void)
{
	return analog[Z_ADDR];
}
//================================================================================
void MegaJoystick_::setXAxisRotation(uint16_t value)
{
	analog[XROT_ADDR] = value;
	if (autoSendState) sendState();
}
uint16_t MegaJoystick_::getXAxisRotation(void)
{
	return analog[XROT_ADDR];
}
//================================================================================
void MegaJoystick_::setYAxisRotation(uint16_t value)
{
	analog[YROT_ADDR] = value;
	if (autoSendState) sendState();
}
uint16_t MegaJoystick_::getYAxisRotation(void)
{
	return analog[YROT_ADDR];
}
//================================================================================
void MegaJoystick_::setZAxisRotation(uint16_t value)
{
	analog[ZROT_ADDR] = value;
	if (autoSendState) sendState();
}
uint16_t MegaJoystick_::getZAxisRotation(void)
{
	return analog[ZROT_ADDR];
}
//================================================================================
// Input = angle in degrees (0-360)
void MegaJoystick_::setHatSwitchDg(int8_t hatSwitchIndex, int16_t angle)
{
	uint8_t value = 0x0F;
	if (angle < 0)	value = 8;
	if (angle >= 0 && angle < 23) value = 0;
	else if (angle < 68) value = 1;
	else if (angle < 113) value = 2;
	else if (angle < 158) value = 3;
	else if (angle < 203) value = 4;
	else if (angle < 245) value = 5;
	else if (angle < 293) value = 6;
	else if (angle < 338) value = 7;
	else if (angle < 360) value = 0;
	
	hatSwitch[hatSwitchIndex % 4] = value;
	if (autoSendState) sendState();
}
//================================================================================
//Input = 4 separate button inputs
// Caution!!! Active low inputs!!!
void MegaJoystick_::setHatSwitch(int8_t hatSwitchIndex, int btnUp, int btnL, int btnDwn, int btnR)
{
	uint8_t input = 0;
	
	if (digitalRead(btnUp) == LOW)	bitSet(input,HAT_BIT_UP);
	else							bitClear(input,HAT_BIT_UP);
	if (digitalRead(btnR) == LOW)	bitSet(input,HAT_BIT_RIGHT);
	else							bitClear(input,HAT_BIT_RIGHT);
	if (digitalRead(btnDwn)==LOW)	bitSet(input,HAT_BIT_DOWN);
	else							bitClear(input,HAT_BIT_DOWN);
	if (digitalRead(btnL) == LOW)	bitSet(input,HAT_BIT_LEFT);
	else							bitClear(input,HAT_BIT_LEFT);
	
	if (input>12)	input = 0;							//only 13 bytes in hat decoder array
	hatSwitch[hatSwitchIndex % 4] = pgm_read_byte_near(&_hat_decoder[input]);
	if (autoSendState) sendState();	
}
//================================================================================
// assuming 2 HATs are connected to one 8bit port/groupped into 8bit byte
// (Right, Down, Left, Up, Hat0 Hat1)
// 7   6   5   4   3   2   1   0
// R1  D1  L1  U1  R0  D0  L0  U0

void MegaJoystick_::set2HatSwitch(int8_t hatSwitchIndex, uint8_t data)
{
	uint8_t index1,index2;
	
	if (hatSwitchIndex>3)	return;
	
	index1 = data&0x0F;
	index2 = (data&0xF0)>>4;
	
	if (index1 > 12) index1 = 0;			//safety first
	if (index2 > 12) index2 = 0;			//decoder array is only 13 bytes long
	
	hatSwitch[hatSwitchIndex] = pgm_read_byte_near(&_hat_decoder[index1]);
	
	if (hatSwitchIndex<3)			
	{
		hatSwitch[hatSwitchIndex+1] = pgm_read_byte_near(&_hat_decoder[index2]);
	}
	if (autoSendState) sendState();	
}
//================================================================================
// assuming 4 HATs are connected to one 16bit port/groupped into 16bit word
// (Right, Down, Left, Up, Hat0 Hat1,Hat2,Hat3)
// F   E   D   C   B   A   9   8   7   6   5   4   3   2   1   0
// R3  D3  L3  U3  R2  D2  L2  U2  R1  D1  L1  U1  R0  D0  L0  U0
void MegaJoystick_::set4HatSwitch(uint16_t data)
{
	uint8_t i, index;
	
	for (i=0; i<4; i++)
	{
		index = data & 0x0F;
		if (index>12)	index = 0;
		hatSwitch[i] = pgm_read_byte_near(&_hat_decoder[index]);
		data >>= 4;
	}
	if (autoSendState) sendState();	
}
//================================================================================
// using gameboy mini joystick as HAT switch, reqzured two ADC inputs  
void MegaJoystick_::setHatSwitchAnalog(int8_t hatSwitchIndex, int8_t xPin, int8_t yPin, uint16_t threshold)
{
	uint16_t x = 0, y = 0;
	uint8_t index = 0;
	x = analogRead(xPin);
	y = analogRead(yPin);
	if (threshold > 10)	threshold = 10;

	threshold = map(threshold,0,10,100,400);
	
	if (x<threshold)				bitSet(index,HAT_BIT_LEFT);			//Left pressed
	else if (x>(1023-threshold))	bitSet(index,HAT_BIT_RIGHT);		//Right pressed
	else
	{
									bitClear(index,HAT_BIT_LEFT);		//both
									bitClear(index,HAT_BIT_RIGHT);		//released
	}
	if (y<threshold)				bitSet(index,HAT_BIT_DOWN);			//down pressed
	else if (y>(1023-threshold)) 	bitSet(index,HAT_BIT_UP);			//up pressed
	else
	{
									bitClear(index,HAT_BIT_DOWN);		//both 
									bitClear(index,HAT_BIT_UP);			//released
	}
	if (index>12)	index = 0;
	hatSwitch[hatSwitchIndex % 4] = pgm_read_byte_near(&_hat_decoder[index]);
	if (autoSendState) sendState();	
}

int16_t MegaJoystick_::getHatSwitch(uint8_t hatSwitchIndex)
{
	int16_t angle = -1;
	
	if (hatSwitch[hatSwitchIndex % 4] != 8) 	angle = hatSwitch[hatSwitchIndex % 4]*45;
	return angle;
}

//================================================================================
/*
	USB data packet:						Addr:
	16 byte = 128 buttons					0x00
	2 bytes	= xAxis							0x10
	2 bytes = yAxis							0x12					
	2 bytes = zAxis							0x14
	2 bytes = xAxisRotation					0x16
	2 bytes = yAxisRotation					0x18
	2 bytes = zAxisRotation					0x1A
	34 bytes= 17x2bytes = 17 sliders		0x1C
	2 bytes = 4x4bit 4 x HAT				0x3E

*/

void MegaJoystick_::sendState()
{
	uint8_t data[JOYSTICK_STATE_SIZE];
	uint8_t i;
	
	//copy 16 bytes with button states
	for (i=BUTTON_DATA_OFFSET;i<16;i++)
	{
		data[i] = buttons[i];
	}
	
	//Axis
	for (i=0;i<24;i++)
	{
		data[(AXIS_DATA_OFFSET+(i<<1))] = analog[i];
		data[(AXIS_DATA_OFFSET+(i<<1)+1)] = analog[i]>>8;
	}
	
	//HAT switches, 2 bytes
	//H3H2H1H0
	
	data[HAT_DATA_OFFSET] = ((hatSwitch[2]&0x0F)<<4)|(0x0F & hatSwitch[3]);
	data[HAT_DATA_OFFSET+1] = ((hatSwitch[0]&0x0F)<<4)|(0x0F & hatSwitch[1]);
	
	// HID().SendReport(Report number, array of values in same order as HID descriptor, length)
	HID().SendReport(JOYSTICK_REPORT_ID, data, JOYSTICK_STATE_SIZE); 
}

MegaJoystick_ Joystick;

#endif