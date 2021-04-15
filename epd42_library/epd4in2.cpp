/**
 *  @filename   :   epd4in2.cpp
 *  @brief      :   Implements for Dual-color e-paper library
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
 *
 *
 * Edited by Deep Tronix, 2021; find me @ rebrand.ly/deeptronix
 */

#include <stdlib.h>
#include <epd4in2.h>

Epd::~Epd(){
};

Epd::Epd(){
  reset_pin = RST_PIN;
  dc_pin = DC_PIN;
  cs_pin = CS_PIN;
  busy_pin = BUSY_PIN;
  
  width = EPD_WIDTH;
  height = EPD_HEIGHT;
};

int Epd::Init(uint8_t M, uint8_t N){
   
	if(IfInit() != 0){	/* this calls the peripheral hardware interface, see epdif */
    return -1;
  }
  
  /* EPD hardware init start */
  Reset();
  SendCommand(POWER_SETTING);
  SendData(0x03);                  // VDS_EN, VDG_EN
  SendData(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
  SendData(0x2b);                  // VDH
  SendData(0x2b);                  // VDL
  SendData(0xff);                  // VDHR
  SendCommand(BOOSTER_SOFT_START);
  SendData(0x17);
  SendData(0x17);
  SendData(0x17);                  //07 0f 17 1f 27 2F 37 2f
  SendCommand(POWER_ON);
  WaitUntilIdle();
  SendCommand(PANEL_SETTING);
	//  SendData(0xbf);    // KW-BF   KWR-AF  BWROTP 0f
  //  SendData(0x0b);
	//	SendData(0x0F);  //300x400 Red mode, LUT from OTP
	//	SendData(0x1F);  //300x400 B/W mode, LUT from OTP
	SendData(0x3F); //300x400, LUT set by register, BW only mode, scan bottom-top, set pixel from left to right on every line
	//	SendData(0x2F); //300x400 Red mode, LUT set by register

  SendCommand(PLL_CONTROL);
  SendData(0x3F & (((M << 3) & 0x38) | (N & 0x7)));        // 3A 100Hz   29 150Hz   39 200Hz    31 171Hz       3C 50Hz (default)    0B 10Hz
  
  updateCurrSpeedCoeff(M, N);
  /* EPD hardware init end */
  return 0;
}


/**
 * @brief: After this command is transmitted, the chip would enter the deep-sleep mode to save power. 
 *         The deep sleep mode would return to standby by hardware reset. The only one parameter is a 
 *         check code, the command would be executed if check code = 0xA5. 
 *         You can use Epd::Reset() to awaken and use Epd::Init() to initialize.
 */
void Epd::Sleep() {
  SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
  SendData(0x17);                       //border floating    
  SendCommand(VCM_DC_SETTING);          //VCOM to 0V
  SendCommand(PANEL_SETTING);
  DelayMs(100);          

  SendCommand(POWER_SETTING);           //VG&VS to 0V fast
  SendData(0x00);        
  SendData(0x00);        
  SendData(0x00);              
  SendData(0x00);        
  SendData(0x00);
  DelayMs(100);          
              
  SendCommand(POWER_OFF);          //power off
  WaitUntilIdle();
  SendCommand(DEEP_SLEEP);         //deep sleep
  SendData(0xA5);
}


void Epd::Wake(uint8_t M, uint8_t N) {
    Reset();
    Init(M, N);
    updateCurrSpeedCoeff(M, N);
}

/**
 *  @brief: module reset. 
 *          often used to awaken the module in deep sleep, 
 *          see Epd::Sleep();
 */
void Epd::Reset(void){
  DigitalWrite(reset_pin, LOW);
  DelayMs(200);
  DigitalWrite(reset_pin, HIGH);
  DelayMs(200); 
}



/**
 *  @brief: transmit partial data to the SRAM.  The final parameter is either dtm=1 and dtm=2 (data transmission mode)
 */
void Epd::SetPartialWindow(const unsigned char* buffer_black, int x, int y, int w, int l, int dtm){
  SendCommand(PARTIAL_IN);
  SendCommand(PARTIAL_WINDOW);
  SendData(x >> 8);	//  Horizontal start, 8th bit : HRST[8]
  SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
  SendData(((x & 0xf8) + w  - 1) >> 8);
  SendData(((x & 0xf8) + w  - 1) | 0x07);
  SendData(y >> 8);
  SendData(y & 0xff);
  SendData((y + l - 1) >> 8);
  SendData((y + l - 1) & 0xff);
  SendData(0x01);         // Gates scan both inside and outside of the partial window (default)
  SendCommand((dtm == 1) ? DATA_START_TRANSMISSION_1 : DATA_START_TRANSMISSION_2);
  if (buffer_black != NULL){
    for(uint16_t i = 0; i < ((w / 8) * l); i++){
      SendData(buffer_black[i]);
    }
  }
	else{
    for(uint16_t i = 0; i < ((w / 8) * l); i++){
      SendData(0x00);
    }
  }
  SendCommand(PARTIAL_OUT);
}


/**
 * @brief: clear the frame data from both SRAMs, this won't refresh the display
 */
void Epd::ClearFrame(void){
  SendCommand(RESOLUTION_SETTING);
  SendData(width >> 8);
  SendData(width & 0xff);
  SendData(height >> 8);
  SendData(height & 0xff);

  SendCommand(DATA_START_TRANSMISSION_1);
  DelayMs(2);
  for(uint16_t i = 0; i < ((width / 8) * height); i++){
    SendData(0xFF);
  }
  DelayMs(2);
  SendCommand(DATA_START_TRANSMISSION_2);
  DelayMs(2);
  for(uint16_t i = 0; i < ((width / 8) * height); i++){
    SendData(0xFF);
  }
  DelayMs(2);
}


/**
 * @brief: refresh and displays the frame
 */
void Epd::DisplayFrame(const unsigned char* frame_buffer) {
  SendCommand(RESOLUTION_SETTING);
  SendData(width >> 8);        
  SendData(width & 0xff);
  SendData(height >> 8);
  SendData(height & 0xff);

  SendCommand(VCM_DC_SETTING);
  SendData(0x12);            // Set VCOM to -0.7V       

  SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
  SendCommand(0x97);    //VBDF 17|D7 VBDW 97  VBDB 57  VBDF F7  VBDW 77  VBDB 37  VBDR B7

  if (frame_buffer != NULL){
    SendCommand(DATA_START_TRANSMISSION_1);
    for(uint16_t i = 0; i < ((width / 8) * height); i++){
      SendData(0xFF);      // bit set: white, bit reset: black
    }
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_2); 
    for(uint16_t i = 0; i < ((width / 8) * height); i++){
      SendData(pgm_read_byte(&frame_buffer[i]));
    }  
    DelayMs(2);                  
  }

  SetLut();

  SendCommand(DISPLAY_REFRESH); 
  DelayMs(100);
  WaitUntilIdle();
}


/**
 *  @brief: set the look-up table
 */
void Epd::SetLut(void) {
  unsigned int count;     
  SendCommand(LUT_FOR_VCOM);                            //vcom
  for(count = 0; count < 44; count++){
    SendData(lut_vcom0[count]);
  }
  
  SendCommand(LUT_WHITE_TO_WHITE);                      //ww --
  for(count = 0; count < 42; count++){
    SendData(lut_ww[count]);
  }   
  
  SendCommand(LUT_BLACK_TO_WHITE);                      //bw r
  for(count = 0; count < 42; count++){
    SendData(lut_bw[count]);
  } 

  SendCommand(LUT_WHITE_TO_BLACK);                      //wb w
  for(count = 0; count < 42; count++){
    SendData(lut_wb[count]);
  } 

  SendCommand(LUT_BLACK_TO_BLACK);                      //bb b
  for(count = 0; count < 42; count++){
    SendData(lut_bb[count]);
  } 
}


/**
 * @brief: This displays the frame data from SRAM
 */
void Epd::DisplayFrame(void){
  SetLut();
  SendCommand(DISPLAY_REFRESH); 
  DelayMs(100);
  WaitUntilIdle();
}








void Epd::DisplayFrameQuick(void){
  SetLutQuick();
  SendCommand(DISPLAY_REFRESH);
}

/**
 *  @brief: set the look-up table for quick display (partial refresh)
 */

void Epd::SetLutQuick(void) {
  unsigned int count;     
  SendCommand(LUT_FOR_VCOM);                            //vcom
  for(count = 0; count < 44; count++){
    SendData(lut_vcom0_quick[count]);
  }
  
  SendCommand(LUT_WHITE_TO_WHITE);                      //ww --
  for(count = 0; count < 42; count++){
    SendData(lut_ww_quick[count]);
  }   
  
  SendCommand(LUT_BLACK_TO_WHITE);                      //bw r
  for(count = 0; count < 42; count++){
    SendData(lut_bw_quick[count]);
  } 

  SendCommand(LUT_WHITE_TO_BLACK);                      //wb w
  for(count = 0; count < 42; count++){
    SendData(lut_wb_quick[count]);
  } 

  SendCommand(LUT_BLACK_TO_BLACK);                      //bb b
  for(count = 0; count < 42; count++){
    SendData(lut_bb_quick[count]);
  } 
}





/**
 *  @brief: sends LUT for "healthy" direct update
 * 	@param: reset_cnt: when true, resets the heavy cycle counter; 
                      that direct update will make use of strong LUTs (B2B & W2W).
 */

void Epd::DisplayFrameQuickAndHealthy(bool reset_cnt){
  SetLutQuickAndHealthy(reset_cnt);
  SendCommand(DISPLAY_REFRESH);
}


void Epd::SetLutQuickAndHealthy(bool reset_cnt){
	uint8_t w2w_repeat, b2b_repeat;
	uint8_t vcom_repeat, b2w_repeat, w2b_repeat;

  uint8_t count;
  static uint8_t weak_cycle_cnt = 0;	// weak cycle counter
  
  if(reset_cnt)  weak_cycle_cnt = 0;
  if(!weak_cycle_cnt){
  	w2w_repeat = 1;
  	b2b_repeat = 1;
  	vcom_repeat = 1;
		b2w_repeat = 2;
		w2b_repeat = 2;
  }
  else{
  	w2w_repeat = 0;
  	b2b_repeat = 0;
  	vcom_repeat = 1;
		b2w_repeat = 2;
		w2b_repeat = 2;
	}
	weak_cycle_cnt = (weak_cycle_cnt + 1) % HEAVY_CYCLE_NR;
	
	SendCommand(LUT_FOR_VCOM);                            //vcom
  for(count = 0; count < 44; count++){
    SendData((count == 5)? vcom_repeat  :  lut_vcom0_quick[count]);	// the 5th byte in the LUT is the one used for repeating the row
  }
  
	SendCommand(LUT_WHITE_TO_WHITE);                      //ww --
  for(count = 0; count < 42; count++){
    SendData((count == 5)? w2w_repeat  :  lut_ww_quick[count]);
  }
  
	SendCommand(LUT_BLACK_TO_WHITE);                      //bw r
  for(count = 0; count < 42; count++){
    SendData((count == 5)? b2w_repeat  :  lut_bw_quick[count]);
  }

	SendCommand(LUT_WHITE_TO_BLACK);                      //wb w
  for(count = 0; count < 42; count++){
    SendData((count == 5)? w2b_repeat  :  lut_wb_quick[count]);
  }

	SendCommand(LUT_BLACK_TO_BLACK);                      //bb b
  for(count = 0; count < 42; count++){
    SendData((count == 5)? b2b_repeat  :  lut_bb_quick[count]);
  }
}



/**
 *  @brief: draws the image using the number of gray shades specified 
 						(but altering the value "SHADES" requires editing 
						 the SetLutShades B2B formula and other LUT values)
		@param: buffer_black: pointer to the image array (each pixel is an 8 bit value between 0-255)
						w, l : the image dimensions. For best output, only use 400x300 images.
 */

#define SHADES 8
void Epd::drawGrayShades(const uint8_t* buffer_black, int w, int l){
	uint8_t m, n, old_m, old_n;
	getCurrSpeedCoeff(m, n);
	if(m != 5  ||  n != 1){
		old_m = m;
		old_n = n;
		Reset();
	  Init(5, 1);		// the waveforms are based on this refresh rate!
	}
	
	uint8_t* vect = (uint8_t*)malloc(8);
	uint8_t cluster = 0;
		
	for(uint8_t shade = 0; shade < (SHADES - 1); shade++){
		uint8_t thresh = 255 - (255*(shade+1))/SHADES;
		
	  SendCommand(PARTIAL_IN);
	  SendCommand(PARTIAL_WINDOW);
	  SendData(0);	   //  Horizontal start, 8th bit : HRST[8]
	  SendData(0);
	  SendData((w  - 1) >> 8);
	  SendData((w  - 1) | 0x07);
	  SendData(0);
	  SendData(0);
	  SendData((l - 1) >> 8);
	  SendData((l - 1) & 0xff);
	  SendData(0x01);         // Gates scan both inside and outside of the partial window (default)
	  SendCommand(DATA_START_TRANSMISSION_2);
	  if (buffer_black != NULL){
	    for(uint32_t i = 0; i < ((w / 8) * l); i++){
	    	
				for(uint8_t off = 0; off < 8; off++){
					uint8_t value = (buffer_black[((i*8)+off)] < thresh)?  0x00 : 0xFF;
					vect[off] = value;
				}
	    	
	    	cluster = byteTo8Bits(vect);
	      SendData(cluster);
	    }
	  }
		else{
	    for(uint16_t i = 0; i < ((w / 8) * l); i++){
	      SendData(0x00);
	    }
	  }
	  
	  SendCommand(PARTIAL_OUT);
	  DisplayFrameShades(shade);
	  WaitUntilIdle();
	}
	
	Reset();
  Init(old_m, old_n);		// restore old refresh rate
}

/**
 *  @brief: sends LUT for gray shade update
 * 	@param: grayshade_cnt: the value is the iteration number in the drawing process
 */

void Epd::DisplayFrameShades(uint8_t grayshade_cnt){
	SetLutShades(grayshade_cnt);
  SendCommand(DISPLAY_REFRESH);
}


void Epd::SetLutShades(uint8_t grayshade_cnt){
  uint8_t b2b_formula = 2 + (15*grayshade_cnt)/(SHADES-1);
  
  unsigned int count;
  SendCommand(LUT_FOR_VCOM);                            //vcom
  for(count = 0; count < 44; count++){
    SendData(lut_vcom0_shade[count]);
  }
  
  SendCommand(LUT_WHITE_TO_WHITE);                      //ww --
  for(count = 0; count < 42; count++){
    SendData(lut_ww_shade[count]);
  }   
  
  SendCommand(LUT_BLACK_TO_WHITE);                      //bw r
  for(count = 0; count < 42; count++){
    SendData(lut_bw_shade[count]);
  } 

  SendCommand(LUT_WHITE_TO_BLACK);                      //wb w
  for(count = 0; count < 42; count++){
    SendData(lut_wb_shade[count]);
  } 

  SendCommand(LUT_BLACK_TO_BLACK);                      //bb b
  for(count = 0; count < 42; count++){
    SendData((count == 1 ||  count == 2)?  b2b_formula : lut_bb_shade[count]);
  } 
}








/**
 *  @brief: basic function for sending commands
 */
void Epd::SendCommand(unsigned char command){
  DigitalWrite(dc_pin, LOW);
  SpiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void Epd::SendData(unsigned char data){
  DigitalWrite(dc_pin, HIGH);
  SpiTransfer(data);
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
void Epd::WaitUntilIdle(void){
  while(DigitalRead(busy_pin) == 0){      //0: busy, 1: idle
    DelayMs(1);
  }
}




uint32_t Epd::index(int x, int y, int w){
  return (x + y*w);
}


uint8_t Epd::byteTo8Bits(uint8_t* source){
	uint8_t output = 0;
	output = (source[0] & 0x80) | ((source[1] >> 1) & 0x40) | ((source[2] >> 2) & 0x20) | ((source[3] >> 3) & 0x10) | 
		((source[4] >> 4) & 0x08) | ((source[5] >> 5) & 0x04) | ((source[6] >> 6) & 0x02) | ((source[7] >> 7) & 0x01);
	return output;
}


void Epd::getCurrSpeedCoeff(uint8_t& m, uint8_t& n){
	m = _curr_M;
	n = _curr_N;
}

void Epd::updateCurrSpeedCoeff(uint8_t m, uint8_t n){		// private function, updated internally
	_curr_M = m;
	_curr_N = n;
}




// WAVEFORM Look-Up-Tables

const unsigned char lut_vcom0[] ={
0x40, 0x17, 0x00, 0x00, 0x00, 0x02,        
0x00, 0x17, 0x17, 0x00, 0x00, 0x02,        
0x00, 0x0A, 0x01, 0x00, 0x00, 0x01,        
0x00, 0x0E, 0x0E, 0x00, 0x00, 0x02,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_vcom0_quick[] ={
0x00, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_vcom0_shade[] ={
0x00, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};







const unsigned char lut_ww[] ={
0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_ww_quick[] ={
0xA0, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_ww_shade[] ={
0xA0, 0x08, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};







const unsigned char lut_bw[] ={
0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};

const unsigned char lut_bw_quick[] ={
0xA0, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};

const unsigned char lut_bw_shade[] ={
0xA0, 0x03, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};







const unsigned char lut_bb[] ={
0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
};

const unsigned char lut_bb_quick[] ={
0x50, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};

const unsigned char lut_bb_shade[] ={
0x50, 0x0A, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};







const unsigned char lut_wb[] ={
0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,         
};

const unsigned char lut_wb_quick[] ={
0x50, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,         
};

const unsigned char lut_wb_shade[] ={
0x50, 0x06, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,         
};



// Ben's (Applied Science) modified LUTs

/*
const unsigned char lut_vcom0_quick[] =
{
0x00, 0x0E, 0x00, 0x00, 0x00, 0x01,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_ww_quick[] ={
0xA0, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_bw_quick[] ={
0xA0, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};

const unsigned char lut_bb_quick[] ={
0x50, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};

const unsigned char lut_wb_quick[] ={
0x50, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,         
};
*/

// EOF


