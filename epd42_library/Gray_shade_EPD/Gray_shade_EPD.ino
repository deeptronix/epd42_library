/*  
 *  Example EPD gray shading test for Teensy (3.5, but should be easily portable to superior boards).
 *  Simply load the SD card with a 400x300 (Width x Height) 24-bit Windows BMP image; name it as
 *  in this sketch variable "filename_prefix" (or do the opposite, actually).
 *  If you want to load other images, be sure never to use more than 8 chars (SD library only supports
 *  up to 8 chars).
 *  You will find the pin definitions inside the library file "epdif.h".
 *  
 *  If you wish to use other resolutions, width and height are set using img_width, img_height variables.
 *  PLEASE NOTE that only images with width multiple of 8 have a chance to work! If other widths are used,
 *  problems will arise from the EPD library.
 *  
 *  If you want to use the ESP32, though I cannot guarantee you will be able to do so (since loading from
 *  SD card uses the same SPI bus as the EPD), you should just need to comment out the definition
 *  "AVR_ARCH" inside the library file "epdif.h".
 *  Also, you cannot load images of 400x300 size, since they won't fit inside the internal RAM
 *  (and accessing the external PSRAM uses, once again, the same shared SPI bus...).
 *  Try, for example, 320x300.
 *  
 *  If you find issues or think you can improve it, please let me know via 
 *  Github (https://github.com/deeptronix/epd42_library)
 *  or at my website (https://rebrand.ly/deeptronix). Thank you!
 */

#define VERBOSE_OUTPUT  true
#define PERFORMANCE_PROFILING  true


#include <SPI.h>
#include <epd4in2.h>  // E-paper main library

#ifdef AVR_ARCH    // for ESP32 boards
  #include <SD.h>
#else
  #include <FS.h>
  #include <SD_MMC.h>
  #define SD SD_MMC
  #define BUILTIN_SDCARD
#endif


Epd epd;

  // EPD paramters:
const uint16_t ep_width = EPD_WIDTH, ep_height = EPD_HEIGHT;    // defined inside "epd4in2.h"; by default, 400x300

  // BMP file name (without extension), NO LONGER than 8 chars; must be a 400x300 (as Width x Height) 24-bit Windows BMP image. Can be color, will be converted to BW anyway.
const String filename_prefix = "grad";
const uint16_t img_width = ep_width, img_height = ep_height;


void setup(){
  Serial.begin(115200);
  delay(2000);    // So that user has enough time to open serial port and see incoming messages
  
  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(BUILTIN_SDCARD)){
    Serial.println(F("failed!"));
    while(1){
      delay(1); // if using a uC with WDT, avoid crashing
    }
  }
  Serial.println(F("OK."));
  
  if (epd.Init(7, 4) != 0) {
    Serial.print("e-Paper init failed!");
    while(1){
      delay(1); // if using a uC with WDT, avoid crashing
    }
  }
  deghost(2);   // parse the number of deghosting cycles to perform. A minimum of 2 is recommended.



  
  static uint8_t img_buffer[img_width * img_height];    // allocate a buffer for the image read from SD card

  char fn_arr[30];
  String filename = filename_prefix + ".bmp";
  filename.toCharArray(fn_arr, filename.length() + 1);
  bmpLoad(fn_arr, 0, 0, img_buffer);

  #if PERFORMANCE_PROFILING
    int32_t q = millis();
  #endif
  
  epd.drawGrayShades(img_buffer, img_width, img_height);
  epd.WaitUntilIdle();
  epd.Sleep();
  
  #if PERFORMANCE_PROFILING
    Serial.println("EPD gray shade drawing took " + String(millis() - q) + "ms.\nStopped.");
  #endif
  
}

void loop(){
}


void deghost(uint8_t cycles){
  epd.ClearFrame();
  for(uint8_t k = 0; k < cycles; k++){
    epd.DisplayFrame();
    delay(500);
  }
}



// eof
