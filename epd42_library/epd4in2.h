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


#define HEAVY_CYCLE_NR  8		// every "HEAVY_CYCLE_NR" of cycles, perform a strong particles drive using W2W and B2B LUTs
	// default 100Hz refresh rate; see page 17 of "4.2inch-e-paper-specification.pdf" for exaplanation
#define default_M 7
#define default_N 2
		

#ifndef EPD4IN2_H
#define EPD4IN2_H

#include "epdif.h"

// Display resolution
#define EPD_WIDTH       400
#define EPD_HEIGHT      300

// EPD4IN2 commands
#define PANEL_SETTING                               0x00
#define POWER_SETTING                               0x01
#define POWER_OFF                                   0x02
#define POWER_OFF_SEQUENCE_SETTING                  0x03
#define POWER_ON                                    0x04
#define POWER_ON_MEASURE                            0x05
#define BOOSTER_SOFT_START                          0x06
#define DEEP_SLEEP                                  0x07
#define DATA_START_TRANSMISSION_1                   0x10
#define DATA_STOP                                   0x11
#define DISPLAY_REFRESH                             0x12
#define DATA_START_TRANSMISSION_2                   0x13
#define LUT_FOR_VCOM                                0x20 
#define LUT_WHITE_TO_WHITE                          0x21
#define LUT_BLACK_TO_WHITE                          0x22
#define LUT_WHITE_TO_BLACK                          0x23
#define LUT_BLACK_TO_BLACK                          0x24
#define PLL_CONTROL                                 0x30
#define TEMPERATURE_SENSOR_COMMAND                  0x40
#define TEMPERATURE_SENSOR_SELECTION                0x41
#define TEMPERATURE_SENSOR_WRITE                    0x42
#define TEMPERATURE_SENSOR_READ                     0x43
#define VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define LOW_POWER_DETECTION                         0x51
#define TCON_SETTING                                0x60
#define RESOLUTION_SETTING                          0x61
#define GSST_SETTING                                0x65
#define GET_STATUS                                  0x71
#define AUTO_MEASUREMENT_VCOM                       0x80
#define READ_VCOM_VALUE                             0x81
#define VCM_DC_SETTING                              0x82
#define PARTIAL_WINDOW                              0x90
#define PARTIAL_IN                                  0x91
#define PARTIAL_OUT                                 0x92
#define PROGRAM_MODE                                0xA0
#define ACTIVE_PROGRAMMING                          0xA1
#define READ_OTP                                    0xA2
#define POWER_SAVING                                0xE3

extern const unsigned char lut_vcom0[];
extern const unsigned char lut_ww[];
extern const unsigned char lut_bw[];
extern const unsigned char lut_bb[];
extern const unsigned char lut_wb[];

extern const unsigned char lut_vcom0_quick[];
extern const unsigned char lut_ww_quick[];
extern const unsigned char lut_bw_quick[];
extern const unsigned char lut_bb_quick[];
extern const unsigned char lut_wb_quick[];

extern const unsigned char lut_vcom0_shade[];
extern const unsigned char lut_ww_shade[];
extern const unsigned char lut_bw_shade[];
extern const unsigned char lut_bb_shade[];
extern const unsigned char lut_wb_shade[];



class Epd : EpdIf {
public:
    unsigned int width;
    unsigned int height;
    
    Epd();
    ~Epd();
    int  Init(uint8_t M = default_M, uint8_t N = default_N);
		void Wake(uint8_t M = default_M, uint8_t N = default_N);
		void Sleep(void);
		void Reset(void);
		
		void SetPartialWindow(const unsigned char* frame_buffer, int x, int y, int w, int l, int dtm = 2);
		void ClearFrame(void);
		void DisplayFrame(const unsigned char* frame_buffer);
		void DisplayFrame(void);
		void SetLut(void);
		
		void DisplayFrameQuick(void);
		void SetLutQuick(void);
		
		void DisplayFrameQuickAndHealthy(bool reset_cnt = false);
		void SetLutQuickAndHealthy(bool reset_cnt);
		
		void drawGrayShades(const uint8_t* buffer_black, int w, int l);
		void DisplayFrameShades(uint8_t grayshade_cnt);
		void SetLutShades(uint8_t grayshade_cnt);
		
		
		void SendCommand(unsigned char command);
    void SendData(unsigned char data);
    void WaitUntilIdle(void);
    
    uint32_t index(int x, int y, int w);
    void getCurrSpeedCoeff(uint8_t& m, uint8_t& n);
		
		// Not implemented in the library:
		/*
		void SetPartialWindowBlack(const unsigned char* buffer_black, int x, int y, int w, int l);
		void SetPartialWindowRed(const unsigned char* buffer_red, int x, int y, int w, int l);
		*/
		
private:
    unsigned int reset_pin;
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int busy_pin;
    
    uint8_t byteTo8Bits(uint8_t* source);
    
    void updateCurrSpeedCoeff(uint8_t m, uint8_t n);
    uint8_t _curr_M, _curr_N;
};

#endif /* EPD4IN2_H */

/* END OF FILE */
