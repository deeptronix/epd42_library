
#define VERBOSE_OUTPUT  true
#define PERFORMANCE_PROFILING  true


#include "z_Setup.h"

#include <SPI.h>
#include <epd4in2.h>  // E-paper main library
#include <Dither.h>

#include <SD.h>
Epd epd;

Dither anim_dt(img_width, img_height);

String filename;
uint8_t cycle_cnt = 0;
uint8_t frame_number = 0;
char fn_arr[30];  // array size is the maximum number of characters allowed for the name, including the file extension

void setup() {
  Serial.begin(115200);
  delay(2000);

  if (epd.Init(7, 4) != 0) {
    Serial.print("e-Paper init failed!");
    while(1){ delay(1); }
  }
  
  for(uint8_t k = 0; k < STARTUP_CLEAR_CY; k++){
    delay(300);
    // Clear the SRAM of the e-paper display
    epd.ClearFrame();
    // Display the data from the SRAM in e-Paper module
    epd.DisplayFrame();
    #if VERBOSE_OUTPUT
      Serial.println("Display deghosted (iter. " + String(k + 1) + " out of " + String(STARTUP_CLEAR_CY) + ").");
    #endif
  }

  epd.Wake(5, 1);

  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(BUILTIN_SDCARD)){
    Serial.println(F("failed!"));
    while(1){ delay(1); }
  }
  Serial.println(F("OK."));
  
}

void loop(){

  // Since this is the same buffer used for picture loading, it MUST be at least the same size as the image!
  static uint8_t img_buffer[img_width * img_height];  // this buffer holds the image as it is from the SD card, aka half resolution
  static uint8_t out_buffer[ep_width * ep_height];    // this buffer holds the dithered and byte-aligned image, ready to be loaded on the EPD
  
  static bool restarted = true;

  uint32_t c = millis();
  filename = filename_prefix + String(frame_number + frame_number_start) + ".bmp";
  filename.toCharArray(fn_arr, filename.length() + 1);
  frame_number = (frame_number + 1) % total_frames;
  #if PERFORMANCE_PROFILING
    uint32_t q = millis();
    static float avg_fr = 0;
  #endif

  bmpLoad(fn_arr, 0, 0, img_buffer);

  #if PERFORMANCE_PROFILING
    Serial.println("              Image loading took " + String(millis() - q) + "ms.");
  #endif

  #if PERFORMANCE_PROFILING
  q = millis();
  #endif

  anim_dt.PersonalFilterDither(img_buffer);
     // If you want, try other dithering filters or techniques
  //anim_dt.AtkinsonDither(img_buffer);
  //anim_dt.fastEDDither(img_buffer);
  //anim_dt.FSDither(img_buffer);
  //anim_dt.thresholding(img_buffer, 130);
  //anim_dt.randomDither(img_buffer, true, 15);

  gray256To8bits(img_buffer, img_width, img_height, out_buffer, IMG_SIZE_FACTOR);    // align byte to 8-bits, for EPD compatibility
  #if PERFORMANCE_PROFILING
  Serial.println("              Dithering and byte-alignment took " + String(millis() - q) + "ms.");
  #endif

  #if PERFORMANCE_PROFILING
  q = millis();
  #endif
  epd.WaitUntilIdle();
  #if PERFORMANCE_PROFILING
    if(millis() - q > 0)  Serial.println("              EPD halted the execution for another " + String(millis() - q) + "ms.");
    else  Serial.println("              EPD didn't slow down the execution (speed bottleneck is elsewhere).");
  #endif

  
  if(frame_number == 0){
    cycle_cnt++;
    #if PERFORMANCE_PROFILING
      avg_fr /= total_frames;
    #endif
    #if VERBOSE_OUTPUT  &&  PERFORMANCE_PROFILING
      Serial.println("\nAnimation cycle completed. Average frame rate was " + String(avg_fr, 2) + " fps.");
    #endif
    #if PERFORMANCE_PROFILING
      avg_fr = 0;
    #endif
  }
  if(cycle_cnt >= ANIMATION_CYCLES){
    epd.Reset();
    epd.Init(7, 4); // set refresh rate at 50Hz ; no need for speed during deghosting.
    #if VERBOSE_OUTPUT
        Serial.println();
    #endif
    for(uint8_t k = 0; k < DEGHOST_CYCLES; k++){
      epd.ClearFrame();
      epd.DisplayFrame();
      #if VERBOSE_OUTPUT
        Serial.println("Display deghosted (iter. " + String(k + 1) + " out of " + String(DEGHOST_CYCLES) + ").");
      #endif
      delay(500);
    }
    cycle_cnt = 0;

    Serial.println("\nGoing to sleep for " + String(SLEEP_TIMEOUT_sec) + " seconds.");
    delay(10);
    
    epd.Sleep();
    delay(SLEEP_TIMEOUT_sec * 1000);
    
    epd.Wake(5, 1); // this will also reset the default (fast) frame rate
    restarted = true;
  }
  else{
    epd.SetPartialWindow(out_buffer, 0, 0, ep_width, ep_height);
    epd.DisplayFrameQuickAndHealthy(restarted);
    restarted = false;
  }
  
  #if PERFORMANCE_PROFILING
    Serial.println("              Current frame rate: " + String(1000.0 / (millis() - c)) + " fps.");
  #endif

  float curr_fr = 1000.0 / (millis() - c);
  #if PERFORMANCE_PROFILING
    avg_fr += curr_fr;
  #endif

}


// EOF
