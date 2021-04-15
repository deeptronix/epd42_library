/**
 *  @filename   :   epdif.cpp
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     August 10 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */



#include "epdif.h"
#include <spi.h>
#include "SPI_adapter.h"	// SPI pin definitions, used for ESP32 implementation


#ifdef SLOWER_SPI_SPEED
#define SPI_SPEED_HZ  1000000
#else
#define SPI_SPEED_HZ  18000000
#endif

EpdIf::EpdIf() {
};

EpdIf::~EpdIf() {
};

void EpdIf::DigitalWrite(int pin, int value){
  digitalWrite(pin, value);
}

int EpdIf::DigitalRead(int pin) {
  return digitalRead(pin);
}

void EpdIf::DelayMs(unsigned int delaytime) {
  delay(delaytime);
}


int EpdIf::IfInit(void) {

	#ifdef AVR_ARCH
  pinMode(CS_PIN, OUTPUT);
  #else
  pinMode(HSPI_SS, OUTPUT); 	// HSPI SS
  #endif
  pinMode(RST_PIN, OUTPUT);
  pinMode(DC_PIN, OUTPUT);
  pinMode(BUSY_PIN, INPUT); 
  

	#ifdef AVR_ARCH
	SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_SPEED_HZ, MSBFIRST, SPI_MODE0));
  #else
	hspi = new SPIClass(HSPI);
	hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS); 	// SCLK, MISO, MOSI, SS
  hspi->beginTransaction(SPISettings(SPI_SPEED_HZ, MSBFIRST, SPI_MODE0));
  #endif
  
  return 0;
}

void EpdIf::SpiTransfer(unsigned char data) {
	#ifdef AVR_ARCH
  	digitalWrite(CS_PIN, LOW);
  #else
  	digitalWrite(HSPI_SS, LOW);
  #endif
  
  #ifdef AVR_ARCH
  SPI.transfer(data);
  #else
  hspi->transfer(data);
  #endif
  
  #ifdef AVR_ARCH
  	digitalWrite(CS_PIN, HIGH);
  #else
  	digitalWrite(HSPI_SS, HIGH);
  #endif
}

	
	
	
	
	
	


