/*
  Joystick.cpp

  Copyright (c) 2015, Matthew Heironimus

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


/*
static const uint8_t _hidReportDescriptor[] PROGMEM = {
  
	// Joystick
	0x05, 0x01,			      // USAGE_PAGE (Generic Desktop)
	0x09, 0x04,			      // USAGE (Joystick)
	0xa1, 0x01,			      // COLLECTION (Application)
	0x85, JOYSTICK_REPORT_ID, //   REPORT_ID (3)

	// 32 Buttons
	0x05, 0x09,			      //   USAGE_PAGE (Button)
	0x19, 0x01,			      //   USAGE_MINIMUM (Button 1)
	0x29, 0x20,			      //   USAGE_MAXIMUM (Button 32)
	0x15, 0x00,			      //   LOGICAL_MINIMUM (0)
	0x25, 0x01,			      //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,			      //   REPORT_SIZE (1)
	0x95, 0x20,			      //   REPORT_COUNT (32)
	0x55, 0x00,			      //   UNIT_EXPONENT (0)
	0x65, 0x00,			      //   UNIT (None)
	0x81, 0x02,			      //   INPUT (Data,Var,Abs)

	// 8 bit Throttle and Steering
	0x05, 0x02,			      //   USAGE_PAGE (Simulation Controls)
	0x15, 0x00,			      //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,	      //   LOGICAL_MAXIMUM (255)
	0xA1, 0x00,			      //   COLLECTION (Physical)
	0x09, 0xBB,			      //     USAGE (Throttle)
	0x09, 0xBA,			      //     USAGE (Steering)
	0x75, 0x08,			      //     REPORT_SIZE (8)
	0x95, 0x02,			      //     REPORT_COUNT (2)
	0x81, 0x02,			      //     INPUT (Data,Var,Abs)
	0xc0,				      //   END_COLLECTION

	// Two Hat switches (8 Positions)
	0x05, 0x01,			      //   USAGE_PAGE (Generic Desktop)
	0x09, 0x39,			      //   USAGE (Hat switch)
	0x15, 0x00,			      //   LOGICAL_MINIMUM (0)
	0x25, 0x07,			      //   LOGICAL_MAXIMUM (7)
	0x35, 0x00,			      //   PHYSICAL_MINIMUM (0)
	0x46, 0x3B, 0x01,	      //   PHYSICAL_MAXIMUM (315)
	0x65, 0x14,			      //   UNIT (Eng Rot:Angular Pos)
	0x75, 0x04,			      //   REPORT_SIZE (4)
	0x95, 0x01,			      //   REPORT_COUNT (1)
	0x81, 0x02,			      //   INPUT (Data,Var,Abs)
                              
	0x09, 0x39,			      //   USAGE (Hat switch)
	0x15, 0x00,			      //   LOGICAL_MINIMUM (0)
	0x25, 0x07,			      //   LOGICAL_MAXIMUM (7)
	0x35, 0x00,			      //   PHYSICAL_MINIMUM (0)
	0x46, 0x3B, 0x01,	      //   PHYSICAL_MAXIMUM (315)
	0x65, 0x14,			      //   UNIT (Eng Rot:Angular Pos)
	0x75, 0x04,			      //   REPORT_SIZE (4)
	0x95, 0x01,			      //   REPORT_COUNT (1)
	0x81, 0x02,			      //   INPUT (Data,Var,Abs)

	// X, Y, and Z Axis
	0x15, 0x00,			      //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,	      //   LOGICAL_MAXIMUM (255)
	0x75, 0x08,			      //   REPORT_SIZE (8)
	0x09, 0x01,			      //   USAGE (Pointer)
	0xA1, 0x00,			      //   COLLECTION (Physical)
	0x09, 0x30,		          //     USAGE (x)
	0x09, 0x31,		          //     USAGE (y)
	0x09, 0x32,		          //     USAGE (z)
	0x09, 0x33,		          //     USAGE (rx)
	0x09, 0x34,		          //     USAGE (ry)
	0x09, 0x35,		          //     USAGE (rz)
	0x95, 0x06,		          //     REPORT_COUNT (6)
	0x81, 0x02,		          //     INPUT (Data,Var,Abs)
	0xc0,				      //   END_COLLECTION
                              
	0xc0				      // END_COLLECTION
};
*/

uint8_t usb_joystick_data[64];

static const uint8_t _hidReportDescriptor[] PROGMEM = 
{
 	0x05, 0x01,                     // Usage Page (Generic Desktop)
	0x09, 0x04,                     // Usage (Joystick)
	0xA1, 0x01,                     // Collection (Application)
	// 128 Buttons
	0x15, 0x00,                     // Logical Minimum (0)
	0x25, 0x01,                     // Logical Maximum (1)
	0x75, 0x01,                     // Report Size (1)
	0x95, 0x80,                     // Report Count (128)
	0x05, 0x09,                     // Usage Page (Button)
	0x19, 0x01,                     // Usage Minimum (Button #1)
	0x29, 0x80,                     // Usage Maximum (Button #128)
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

MegaJoystick_::MegaJoystick_()
{
	// Setup HID report structure
	static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
	HID().AppendDescriptor(&node);
	
	// Initalize State
	xAxis = 0;
	yAxis = 0;
	zAxis = 0;
	xAxisRotation = 0;
	yAxisRotation = 0;
	zAxisRotation = 0;
	buttons[0] = 0;
	buttons[1] = 0;
	buttons[2] = 0;
	buttons[3] = 0;
	buttons[4] = 0;
	buttons[5] = 0;
	buttons[6] = 0;
	buttons[7] = 0;
	buttons[8] = 0;
	buttons[9] = 0;
	buttons[10] = 0;
	buttons[11] = 0;
	buttons[12] = 0;
	buttons[13] = 0;
	buttons[14] = 0;
	buttons[15] = 0;	
	slider[0] = 0;
	slider[1] = 0;
	slider[2] = 0;
	slider[3] = 0;
	slider[4] = 0;
	slider[5] = 0;
	slider[6] = 0;
	slider[7] = 0;
	slider[8] = 0;
	slider[9] = 0;
	slider[10] = 0;
	slider[11] = 0;
	slider[12] = 0;
	slider[13] = 0;
	slider[14] = 0;
	slider[15] = 0;
	slider[16] = 0;
	//throttle = 0;
	//rudder = 0;
	hatSwitch[0] = -1;
	hatSwitch[1] = -1;
	hatSwitch[2] = -1;
	hatSwitch[3] = -1;
}

void MegaJoystick_::begin(bool initAutoSendState)
{
	autoSendState = initAutoSendState;
	sendState();
}

void MegaJoystick_::end()
{
}

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

/*
	button array = 16x8bit
	input = 0-127 >> 5 = 0-15 array index
	bit mask = 0x07 

*/
void MegaJoystick_::pressButton(uint8_t button)
{
	if (button>127)	return;
	bitSet(buttons[button>>3], button&0x07);	//
	if (autoSendState) sendState();
}
void MegaJoystick_::releaseButton(uint8_t button)
{
	if (button>127)	return;
	bitClear(buttons[button>>3], button&0x07);
	if (autoSendState) sendState();
}

void MegaJoystick_::setSlider(uint8_t sliderNo, uin16_t value)
{
	if (sliderNo > 16) return;
	slider[sliderNo] = value;
}


void MegaJoystick_::setXAxis(uint16_t value)
{
	xAxis = value;
	if (autoSendState) sendState();
}
void MegaJoystick_::setYAxis(uint16_t value)
{
	yAxis = value;
	if (autoSendState) sendState();
}
void MegaJoystick_::setZAxis(uint16_t value)
{
	zAxis = value;
	if (autoSendState) sendState();
}

void MegaJoystick_::setXAxisRotation(uint16_t value)
{
	xAxisRotation = value;
	if (autoSendState) sendState();
}
void MegaJoystick_::setYAxisRotation(uint16_t value)
{
	yAxisRotation = value;
	if (autoSendState) sendState();
}
void MegaJoystick_::setZAxisRotation(uint16_t value)
{
	zAxisRotation = value;
	if (autoSendState) sendState();
}

void MegaJoystick_::setHatSwitch(int8_t hatSwitchIndex, int16_t angle)
{
	uint16_t value = 0x0F;
	if (angle > 0 && angle < 23) value = 0;
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
	uint16_t *ptr_start;
	
	//copy 16 bytes with button states
	for (i=BUTTON_DATA_OFFSET;i<16;i++)
	{
		data[i] = buttons[i];
	}
	
	//Axis
	ptr_start = (uint16__t *)(&data[AXIS_DATA_OFFSET]);
	pasteAnalog16(0, xAxis,ptr_start); 				//xAxis
	pasteAnalog16(1, yAxis,ptr_start); 				//yAxis
	pasteAnalog16(2, zAxis,ptr_start); 				//zAxis
	pasteAnalog16(3, xAxisRotation,ptr_start); 		//xAxis
	pasteAnalog16(4, yAxisRotation,ptr_start); 		//yAxis
	pasteAnalog16(5, zAxisRotation,ptr_start); 		//zAxis
	//sliders
	ptr_start = (uint16__t *)(&data[SLIDER_DATA_OFFSET]);
	for (i = 0, i<17, i++)
	{
		pasteAnalog16(i, slider[i],ptr_start); 	
	}
	//HAT switches, 2 bytes
	//H3H2H1H0
	
	data[HAT_DATA_OFFSET] = (hatSwitch[3]<<4)|(0x0F & hatSwitch[2]);
	data[HAT_DATA_OFFSET+1] = (hatSwitch[1]<<4)|(0x0F & hatSwitch[0]);
	
	// HID().SendReport(Report number, array of values in same order as HID descriptor, length)
	HID().SendReport(JOYSTICK_REPORT_ID, data, JOYSTICK_STATE_SIZE); 
}

MegaJoystick_ Joystick;

#endif