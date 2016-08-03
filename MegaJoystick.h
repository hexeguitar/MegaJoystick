/*
  ExtrJoystick.h

  copyright (c) 2016, Piotr Zapart
  
  based on/inspired by:  

  - Arduino Joystick library Copyright (c) 2015, Matthew Heironimus
  - Teensy Extreme Joystick library by Paul Stoffregen/PJRC.com
  
  upgraded to   
				128 buttons @ 1bit
				6 axis @ 16bit
				17 sliders @ 16bit
				4 hat swiches @4bit
	
  Default autoSendState is OFF
  input analog values are uint16_t 

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

#define SLIDER_ADDR				0x06
#define AXIS_ADDR				0x00		

#define X_ADDR					0x00
#define Y_ADDR					0x01
#define Z_ADDR					0x02
#define XROT_ADDR				0x03
#define YROT_ADDR				0x04
#define ZROT_ADDR				0x05
	
//================================================================================
//================================================================================
//  Joystick 

class MegaJoystick_
{
private:
	bool     	 autoSendState;
	uint16_t 	 analog[23];
	uint8_t 	 buttons[16];		//128 buttons
	uint8_t	 	 hatSwitch[4];


public:
	MegaJoystick_();

	void begin(bool initAutoSendState = false);
	void end();

	void setAutoSend(bool state);
	
	void setXAxis(uint16_t value);
	void setYAxis(uint16_t value);
	void setZAxis(uint16_t value);

	void setXAxisRotation(uint16_t value);
	void setYAxisRotation(uint16_t value);
	void setZAxisRotation(uint16_t value);
	
	void setButton(uint8_t button, uint8_t value);
	void pressButton(uint8_t button);
	void releaseButton(uint8_t button);
	
	void setButtonBank8(uint8_t bank, uint8_t buttonStates);
	void setButtonBank16(uint8_t bank, uint16_t buttonStates);		//useful when reading out data from shift registers

	void setSlider(uint8_t sliderNo, uint16_t value);
	
	void setHatSwitchDg(int8_t hatSwitchIndex, int16_t angle);
	void setHatSwitch(int8_t hatSwitchIndex, int btnUp, int btnL, int btnDwn, int btnR);
	void set2HatSwitch(int8_t hatSwitchIndex, uint8_t data);
	void set4HatSwitch(uint16_t data);
	void setHatSwitchAnalog(int8_t hatSwitchIndex, int8_t xPin, int8_t yPin, uint16_t threshold);

	uint16_t getXAxis(void);
	uint16_t getYAxis(void);
	uint16_t getZAxis(void);
	
	uint16_t getXAxisRotation(void);
	uint16_t getYAxisRotation(void);
	uint16_t getZAxisRotation(void);
	
	bool getButton(uint8_t button);
	int16_t getHatSwitch(uint8_t hatSwitch);
	uint16_t getSlider(uint8_t sliderNo);
	

	void sendState();
};
extern MegaJoystick_ Joystick;

#endif
#endif
