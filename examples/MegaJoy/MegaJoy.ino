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
  
  D7 (INT6) - MCP23017 INTA+INTB (set to open collector)  
  
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


#include <arduino.h>
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

//3 rodzaje odczytywania stanu  przyciskow
// pojedynczo, w 8 bitowym bloku i 16 bitowym bloku
// odkomentowac jedna z ponizszych linii
//#define BUTTON_SINGLE_READ
//#define BUTTON_8BIT_READ
#define BUTTON_16BIT_READ



void updateJoyButtons(void)
{

	
	// czytanie stanu przyciskow pojedynczo
#ifdef BUTTON_SINGLE_READ

	uint8_t i;
	uint16_t btnBank[8];
	
	btnBank[0] = readMCP23017(MCP1_SLAVE_ADDR);
	//btnBank[1] = readMCP23017(MCP2_SLAVE_ADDR);
	// ...
	// btnBank[7] = readMCP23017(MCP8_SLAVE_ADDR);
	
	Joystick.setAutoSend(false);		//wylaczamy autosend, bedzie wyslany jeden raz na koncu

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
#endif

	// 8 bitowy blok, dla przykladu jeden z portow pierwszego MCO23017
#ifdef 	BUTTON_8BIT_READ
	Joystick.setButtonBank8(0,readMCP23017(MCP1_SLAVE_ADDR));		//zapisuje bank 0
	Joystick.setButtonBank8(1,(readMCP23017(MCP1_SLAVE_ADDR))>>8);	//zapisuje bank 1
	//setButtonBank8(2,readMCP23017(MCP2_SLAVE_ADDR));		//zapisuje bank 0
	//setButtonBank8(3,(readMCP23017(MCP2_SLAVE_ADDR))>>8);	//zapisuje bank 1
#endif

	// 16 bitowy blok
#ifdef 	BUTTON_16BIT_READ
	Joystick.setButtonBank16(0,readMCP23017(MCP1_SLAVE_ADDR));		//zapisuje bank 0 i 1
	//setButtonBank16(0,readMCP23017(MCP2_SLAVE_ADDR));		//zapisuje bank 2 i 3
#endif

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
	
	drawUARTbargraph(13,9, Joystick.getXAxis());
	drawUARTbargraph(14,9, Joystick.getYAxis());
	drawUARTbargraph(15,9, Joystick.getZAxis());
	drawUARTbargraph(16,9, Joystick.getXAxisRotation());
	drawUARTbargraph(17,9, Joystick.getYAxisRotation());
	drawUARTbargraph(18,9, Joystick.getZAxisRotation());
	drawUARTbargraph(19,9, Joystick.getSlider(0));
	drawUARTbargraph(20,9, Joystick.getSlider(1));
	term.position(21,9); term.print(Joystick.getHatSwitch(0));term.print(F("   "));
	term.position(22,9); term.print(Joystick.getHatSwitch(1));term.print(F("   "));
	term.position(23,9); term.print(Joystick.getHatSwitch(2));term.print(F("   "));
	term.position(24,9); term.print(Joystick.getHatSwitch(3));term.print(F("   "));
	
	
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
	
	term.position(12,0);
	term.println(F("Axis:\t"));
	term.println(F("X=\t"));
	term.println(F("Y=\t"));
	term.println(F("Z=\t"));
	term.println(F("Xrot=\t"));
	term.println(F("Yrot=\t"));
	term.println(F("Zrot=\t"));
	term.println(F("Slider0=\t"));
	term.println(F("Slider1=\t"));
	term.println(F("HAT0=\t"));
	term.println(F("HAT1=\t"));
	term.println(F("HAT2=\t"));
	term.println(F("HAT3=\t"));
	
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
	// INT6 
	pinMode(7,INPUT_PULLUP);
	digitalWrite(7,HIGH);
	EICRB |= (1<<ISC61);//|(1<<ISC60); 	// wyzwalane zboczem opadajacym
	EIMSK |= (1<<INT6); 				// uruchom przerwanie
  
	//Piny uzyte go testowania HAT bez uzycia MCP23017
	pinMode(6,INPUT_PULLUP);
	pinMode(5,INPUT_PULLUP);
	pinMode(4,INPUT_PULLUP);
	pinMode(3,INPUT_PULLUP);
  
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
	Joystick.setYAxisRotation((analogRead(1)<<6)+8192);	//Yrot = analog 0 przesuniete o 8192
	Joystick.setZAxisRotation((analogRead(2)<<6)+8192);	//Zrot = analog 0 przesuniete o 8192
	
	Joystick.setSlider(0,(analogRead(3)<<6));
	Joystick.setSlider(1,(analogRead(3)<<6));

	
	// ##########  HATs ###########
	// 5 sposobow uaktualniania stanu HATow
	// odkomentowac jedna z ponizszych deklaracji 
//#define HAT_TEST_DEGREE
//#define HAT_TEST_BUTTON_LIST
//#define HAT_TEST_8BIT
//#define HAT_TEST_16BIT	
#define HAT_TEST_ANALOG
	
#ifdef HAT_TEST_DEGREE	
	//### Metoda 1 ### 
	//Jako parametr wejsciowy funkcja przyjmuje numer HATa (0-3) i kat w stopniach, (zakres 0-360, -1 odpowiada OFF)
	//przyklad czyta analoghowa wartosc wejscia A0, mapuje ja na zakres -1 ... 360 i ustawia HATy 
	int16_t angle = analogRead(0);
	angle = map(angle,0,1023,0,360);
	if (angle < 5)	angle = -1;

		Joystick.setHatSwitchDg(0,angle);
	
#endif
#ifdef HAT_TEST_BUTTON_LIST
	//### Metoda 2 ###
	// funkcja przyjmuje numer HATa i nuery pinow czterech przyciskow w kolejnowsci gora, prawo, dol, lewo
	// pamietac o odpowiednim ustawieniu wejsc! input+pullup
	// niezbyt efektywna w metoda, uzywac jesli nie ma innej opcji
	Joystick.setHatSwitch(0,6,5,4,3);
	Joystick.setHatSwitch(1,6,5,4,3);
	Joystick.setHatSwitch(2,6,5,4,3);
	Joystick.setHatSwitch(3,6,5,4,3);
#endif	
#ifdef HAT_TEST_8BIT
	//### Metoda 3 ###
	// sluzy do jednoczesnego odcyztania dwoch HATow podlaczonych do 8bitowego portu
	// np polowa MCP23017.
	//	Zakladamy, ze 2 HATy podlaczone sa do jednego MCP23017 w sposob opisany ponizej:
	//(Right, Down, Left, Up, Hat0 Hat1)
	// 7   6   5   4   3   2   1   0
	// R1  D1  L1  U1  R0  D0  L0  U0
	Joystick.setHatSwitch(0,readMCP23017(MCP1_SLAVE_ADDR));		//wpisane beda HAT0 i HAT1
	Joystick.setHatSwitch(2,readMCP23017(MCP1_SLAVE_ADDR)>>8);	// HAT2 i HAT3
#endif
#ifdef HAT_TEST_16BIT
	//### Metoda 4 ###
	//Zakladamy, ze 4 HATy podlaczone sa do jednego MCP23017 w sposob opisany ponizej:
	//(Right, Down, Left, Up, Hat0 Hat1,Hat2,Hat3)
	//F   E   D   C   B   A   9   8   7   6   5   4   3   2   1   0
	//R3  D3  L3  U3  R2  D2  L2  U2  R1  D1  L1  U1  R0  D0  L0  U0
	Joystick.set4HatSwitch(readMCP23017(MCP1_SLAVE_ADDR));
#endif
#ifdef HAT_TEST_ANALOG
	//### Metoda 5 ###
	//analogowy mini joystick uzyty jako HAT switch. Wymagane dwa wejscia analogowe
	//argumenty: 
	//numer HATa, analogowy pin osi X (lewo/prawo), analogowy pin osi Y (gora/dol), prog zadzialania 0-10
	Joystick.setHatSwitchAnalog(0,0,1,8);	//HAT0, A0=x, A1=y, prog = 8
	

#endif
	
	
	
#ifdef DEBUG_UART	
	sendUARTreport();
#endif
}

