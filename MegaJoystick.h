/*
  ExtrJoystick.h

  Copyright (c) 2015, Matthew Heironimus
  
  upgraded to   
				128 buttons @ 1bit
				6 axis @ 16bit
				17 sliders @ 16bit
				4 hat swiches @4bit
				
	by Piotr Zapart, 07-08 2016

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

#ifndef MEGAJOYSTICK_h
#define MEGAJOYSTICK_h

#include "HID.h"

#if ARDUINO < 10606
#error The Joystick library requires Arduino IDE 1.6.6 or greater. Please update your IDE.
#endif

#if !defined(USBCON)
#error The Joystick library can only be used with a USB MCU (e.g. Arduino Leonardo, Arduino Micro, etc.).
#endif

#if !defined(_USING_HID)

#warning "Using legacy HID core (non pluggable)"

#else

#define BUTTON_DATA_OFFSET		0x00
#define AXIS_DATA_OFFSET		0x10
#define SLIDER_DATA_OFFSET		0x1C
#define HAT_DATA_OFFSET			0x3E

	
	
//================================================================================
//================================================================================
//  Joystick (Gamepad)

class MegaJoystick_
{
private:
	bool     	 autoSendState;
	uint16_t	 xAxis;
	uint16_t	 yAxis;
	uint16_t	 zAxis;
	uint16_t	 xAxisRotation;
	uint16_t	 yAxisRotation;
	uint16_t	 zAxisRotation;
	uint16_t 	 slider[17];		//17 sliders @16 bit
	uint8_t 	 buttons[16];		//128 buttons
	uint8_t	 	 hatSwitch[4];

	void pasteAnalog16(unsigned int num, unsigned int value, uint16_t *data_ptr) 
	{
		if (value > 0xFFFF) value = 0xFFFF;
		//uint16_t *p = (uint16_t *)(&usb_joystick_data[ANALOG16_DATA_OFFSET]);
		data_ptr[num] = value;
		if (autoSendState) sendState();
	}
	
public:
	MegaJoystick_();

	void begin(bool initAutoSendState = false);
	void end();

	void setXAxis(uint16_t value);
	void setYAxis(uint16_t value);
	void setZAxis(uint16_t value);

	void setXAxisRotation(uint16_t value);
	void setYAxisRotation(uint16_t value);
	void setZAxisRotation(uint16_t value);

	void setButton(uint8_t button, uint8_t value);
	void pressButton(uint8_t button);
	void releaseButton(uint8_t button);

	void setSlider(uint8_t slider, uint16_t value);
	
	void setHatSwitch(int8_t hatSwitch, int16_t angle);

	void sendState();
};
extern MegaJoystick_ Joystick;

#endif
#endif
