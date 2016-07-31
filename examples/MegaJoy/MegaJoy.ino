/*
  MegaJoystick library test
  for ATMEGA32U4 based Arduino boards
  
  copyright (c) 2016, Piotr Zapart
  
  128 buttons
  6 16bit axis
  17 16bit sliders
  4 HATs
  
  Connections:
  
  UART (Serial1) use VT100 terminal emulator like PuTTy
  D0-2k2-AdapterTX
  D1-2k2-AdapterRX
  
  I2C:
  MCP23017 SCL - Arduino SCL -2k2-|
                                  |-Vcc (pull ups)
  MCP23017 SDA - Arduino SDA -2k2-|
  
  D6 (INT6) - MCP23017 INTA+INTB (set to open collector)  
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/



#include <MegaJoystick.h>
#include <Wire.h>
#include <BasicTerm.h>

#define DEBUG_UART

const int MCP1_SLAVE_ADDR = 0x20;
const int MCP2_SLAVE_ADDR = 0x27;

bool btnUpdateRequest = true;

// ### deklaracje funkcji ###
void initMCP23017(uint8_t slaveAddr);     	//deklaracja funkcji inicjalizujacej 
uint16_t readMCP23017(int8_t slaveAddr);   //deklaracja funkcji odczytywania stanu portow
void  sendUARTreport(void);		//funkcja wysylajaca raport na terminal typu VT100 (np putty)
void updateJoyButtons(void);			//
void drawUARTstartup(void);
void drawUARTbargraph(uint8_t column, uint8_t row, uint16_t value);

BasicTerm term(&Serial1);

// -------------------------------------------------------------------------------------
/*
	Funkcja inicjalizujaca uklad MCP23017
	wszystkie piny jako input + pullup
	argument: adres I2C
	BANK0 = 0 - wykorzystamy sekwencyjny zapis, 
	adres jest automatycznie inkrementowany
	po kazdej operacji zapisu bajtu
*/
void initMCP23017(uint8_t slaveAddr)
{
	// IODIRA -> IODIRB
	Wire.beginTransmission(slaveAddr);
	Wire.write(0x00);       //IODIRA
	Wire.write(0xFF);       //portA = wejscia, adres skacze do 0x01=IODIRB
	Wire.write(0xFF);       //0x01 IODIRB = wejscia
	Wire.write(0xFF); 	  	//0x02 IPOLA = odwrotna polaryzacja, przycisk wcisniety = 1
	Wire.write(0xFF); 		//0x03 IPOLB = jw.
	Wire.endTransmission();

	// Pull-ups
	Wire.beginTransmission(slaveAddr);
	Wire.write(0x0C);       //GPPUA
	Wire.write(0xFF);       //podciagnij wejscia A do VCC,nowy adres = 0x0D=GPPUB
	Wire.write(0xFF);       //podciagnij wejscia B do VCC
	Wire.endTransmission();

	//INTA & INTB - aktywne i otwarty kolektor
	Wire.beginTransmission(slaveAddr);
	Wire.write(0x04);			//GPINTENA
	Wire.write(0xFF); 
	Wire.write(0xFF); 
	Wire.endTransmission();

	Wire.beginTransmission(slaveAddr);
	Wire.write(0x0A);       	//IOCON, dla BANK = 0
	Wire.write(_BV(2)|_BV(6));  //ODR = 1, MIRROR = 1
	Wire.endTransmission();
  
} 
  
// -------------------------------------------------------------------------------------
// Funkcja odczytujaca stany portow GPIOA i GPIOB
// stan zwracany jest w postaci 16bitowego slowa,
// gdzie jeden bit odpowiada jednemu przyciskowi

uint16_t readMCP23017(int8_t slaveAddr)
{
  uint8_t output[2];
  uint8_t index = 0;
  
  Wire.beginTransmission(slaveAddr);    //I2C Start, wyslij adress
  Wire.write(0x12);             		//ustaw wewnetrzny adres na GPIOA
  Wire.endTransmission();         		//I2C stop
  Wire.beginTransmission(slaveAddr);    //I2C Start, wyslij adres
  Wire.requestFrom(slaveAddr,2,false);   //grzecznie popros o 2 bajty
										//po odczytaniu GPIOA wewnetrzny adres 
										//automatycznie zostanie zwiekszony o 1. 
										//czyli wyladuje na GPIOB
										//"true" na koncu generuje I2C stop i zwalnia 
										//magistrale, Wire.endTrasmission nie jest potrzebny
  while (Wire.available())  
  {
    output[index] = Wire.read();    //wpisz GPIOA i B do tablicy
    index++;
  }
  Wire.endTransmission();
  return uint16_t(output[0] | ((output[1])<<8));  //zgrupuj oba bajty w 16 bitowe slowo
} 
// -------------------------------------------------------------------------------------
/*
	128 buttons, 16 buttons pro MCP23017 = 8 banks
*/

void updateJoyButtons(void)
{
	uint8_t i;
	uint16_t btnBank[8];
	
	btnBank[0] = readMCP23017(MCP1_SLAVE_ADDR);
	//btnBank[1] = readMCP23017(MCP2_SLAVE_ADDR);
	// ...
	// btnBank[7] = readMCP23017(MCP8_SLAVE_ADDR);
	
	Joystick.setAutoSend(false);
	
	for (i=0; i<16; i++)
	{
		if (btnBank[0] & (1<<i))	Joystick.setButton(i,1);
		else						Joystick.setButton(i,0);
		if (btnBank[1] & (1<<i))	Joystick.setButton(i+16,1);
		else						Joystick.setButton(i+16,0);
		//...
		// 	if (btnBank[2] & (1<<i)) 	Joystick.setButton(i+32,1);
		//	else						Joystick.setButton(i+32,0);
		// 	if (btnBank[3] & (1<<i)) 	Joystick.setButton(i+48,1);
		//	else						Joystick.setButton(i+48,0);
		// 	if (btnBank[4] & (1<<i)) 	Joystick.setButton(i+64,1);
		//	else						Joystick.setButton(i+64,0);
		// 	if (btnBank[5] & (1<<i)) 	Joystick.setButton(i+80,1);
		//	else						Joystick.setButton(i+80,0);
		// 	if (btnBank[6] & (1<<i)) 	Joystick.setButton(i+96,1);
		//	else						Joystick.setButton(i+96,0);
		// 	if (btnBank[7] & (1<<i)) 	Joystick.setButton(i+112,1);
		//	else						Joystick.setButton(i+112,0);
	}
	Joystick.sendState();
	Joystick.setAutoSend(true);
}
// -------------------------------------------------------------------------------------
void drawUARTbargraph(uint8_t row, uint8_t column, uint16_t value)
{
	char bar[17];
	uint8_t i;
	term.position(row,column);
	for (i=0;i<16;i++)
	{
		if ((value>>12)>i) 	bar[i] = '#';
		else				bar[i] = ' ';
	}
	bar[16] = '\0';
	term.print(bar);
}


// -------------------------------------------------------------------------------------
void  sendUARTreport(void)
{
	uint8_t i,j;
	
	//Buttons
	//j = bank number
	for (j=0;j<8;j++)
	{
		term.position(j+3,0);
		for (i=0;i<16;i++)
		{
			if (Joystick.getButton((j<<4)+i))	//j*16+i=button No
			{
				term.set_color(BT_RED, BT_BLACK);
				term.write('1');
			}
			else
			{
				term.set_color(BT_CYAN, BT_BLACK);
				term.write('0');
			}
			term.write(' ');	
		}
		term.print("\t");
	}
	
	term.set_color(BT_WHITE, BT_BLACK);
	
	drawUARTbargraph(12,9, Joystick.getXAxis());
	drawUARTbargraph(13,9, Joystick.getYAxis());
	drawUARTbargraph(14,9, Joystick.getZAxis());
	drawUARTbargraph(15,9, Joystick.getXAxisRotation());
	drawUARTbargraph(16,9, Joystick.getYAxisRotation());
	drawUARTbargraph(17,9, Joystick.getZAxisRotation());
	drawUARTbargraph(18,9, Joystick.getSlider(0));
	drawUARTbargraph(19,9, Joystick.getSlider(1));
}
// -------------------------------------------------------------------------------------
void drawUARTstartup(void)
{
	// puTTY terminal
	term.init();
    term.cls();
    term.show_cursor(false);		
	term.position(0,0);
	term.set_attribute(BT_NORMAL);
    term.set_attribute(BT_BOLD);
    term.print(F("Arduino Joystick Monitor\t"));
	term.set_attribute(BT_NORMAL);
	term.position(1,0);
	term.print(F("Button status:\t"));
	term.position(2,0);
	term.print(F("0 1 2 3 4 5 6 7 8 9 A B C D E F\t"));
	
	term.position(11,0);
	term.println(F("Axis:\t"));
	term.println(F("X=\t"));
	term.println(F("Y=\t"));
	term.println(F("Z=\t"));
	term.println(F("Xrot=\t"));
	term.println(F("Yrot=\t"));
	term.println(F("Zrot=\t"));
	term.println(F("Slider0=\t"));
	term.println(F("Slider1=\t"));
}
// -------------------------------------------------------------------------------------
// INT6 interrupt, ktos cos wcisnal
ISR(INT6_vect) 
{
  btnUpdateRequest = true;
  EIMSK &= ~(1<<INT6); 				// wylacz przerwanie
}
// -------------------------------------------------------------------------------------

void setup() 
{
	// INT6 - zaimplementowany "na piechote", attachInterrupt() go nie obsluguje
	pinMode(7,INPUT_PULLUP);
	digitalWrite(7,HIGH);
	EICRB |= (1<<ISC61);//|(1<<ISC60); 	// wyzwalane zboczem opadajacym
	EIMSK |= (1<<INT6); 				// uruchom przerwanie
  
	Serial1.begin(115200);
	while (!Serial1);
	Joystick.begin();
	Wire.begin();

#ifdef DEBUG_UART	
	drawUARTstartup();
#endif	
	initMCP23017(MCP1_SLAVE_ADDR);
	//initMCP23017(MCP2_SLAVE_ADDR);
	
	Joystick.setAutoSend(true);
}

void loop() 
{
	//odczytaj przyciski jesli sa nowe dane
	
	if (btnUpdateRequest == true)
	{
		updateJoyButtons();
		btnUpdateRequest = false;
		EIMSK |= (1<<INT6); 				// ponownie uruchom przerwanie INT6
	}

	//analog inputs
	Joystick.setXAxis(analogRead(0)<<6);	//X = analog 0
	Joystick.setYAxis(analogRead(1)<<6);	//X = analog 0
	Joystick.setZAxis(analogRead(2)<<6);	//X = analog 0
	
	Joystick.setXAxisRotation((analogRead(0)<<6)+8192);	//Xrot = analog 0 przesuniete o 8192
	Joystick.setYAxisRotation((analogRead(1)<<6)+8192);	//Xrot = analog 0 przesuniete o 8192
	Joystick.setZAxisRotation((analogRead(2)<<6)+8192);	//Xrot = analog 0 przesuniete o 8192
	
	Joystick.setSlider(0,(analogRead(3)<<6));
	Joystick.setSlider(1,(analogRead(3)<<6));
	

	
#ifdef DEBUG_UART	
	sendUARTreport();
#endif
}

