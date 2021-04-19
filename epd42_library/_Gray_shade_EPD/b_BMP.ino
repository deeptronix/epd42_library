// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel). Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster. 100 pixels seems a
// good balance.

#define BUFFPIXEL 100

void bmpLoad(char *filename, int x, int y, uint8_t* im_buffer) {

  File     bmpFile;
  int16_t  bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint32_t buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  uint16_t w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0;
  uint32_t imgidx = 0;

  if((x >= ep_width) || (y >= ep_height)) return;

  #if VERBOSE_OUTPUT
    Serial.println();
    Serial.print(F("Loading image '"));
    Serial.print(filename);
    Serial.println('\'');
  #endif
  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.println(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
     uint32_t fsize = read32(bmpFile);
    #if VERBOSE_OUTPUT
      Serial.print(F("File size: "));
      Serial.println(fsize);
    #endif
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    #if VERBOSE_OUTPUT
      Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    #endif
    // Read DIB header
    uint16_t head_sz = read32(bmpFile);
    #if VERBOSE_OUTPUT
      Serial.print(F("Header size: ")); Serial.println(head_sz);
    #endif
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    writeBmpSize(bmpWidth, bmpHeight);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      #if VERBOSE_OUTPUT
        Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      #endif
      if((bmpDepth == 24) && (read32(bmpFile) == 0)){ // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        #if VERBOSE_OUTPUT
          Serial.print(F("Image size: "));
          Serial.print(bmpWidth);
          Serial.print('x');
          Serial.println(bmpHeight);
        #endif

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= ep_width)  w = ep_width  - x;
        if((y+h-1) >= ep_height) h = ep_height - y;

        for (row=0; row<h; row++) { // For each scanline...
          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++){ // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)){ // Indeed
              #if debugging
                p = micros();
              #endif
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
              #if debugging
                p = micros() - p;
              #endif
              #if debugging
                Serial.println(F("Took: ") + String(p) + "us.");
              #endif
            }

            // Convert pixel from BMP to 8-bit array format
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            im_buffer[imgidx++] = color888ToGray256(r, g, b);
            //img_buffer[imgidx++] = convert.color888To332(r, g, b);
            
          } // end pixel
        } // end scanline
      } // end
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));
}


uint8_t color888ToGray256(uint8_t r, uint8_t g, uint8_t b) {
  return ((r + g + b) / 3);
}



// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}



void writeBmpSize(uint16_t w, uint16_t h){
  _BmpSize(w, h, true);
}

uint16_t readBmpWidth(){
  uint16_t w, h;
  _BmpSize(w, h, false);
  return w;
}

uint16_t readBmpHeight(){
  uint16_t w, h;
  _BmpSize(w, h, false);
  return h;
}

void _BmpSize(uint16_t &w, uint16_t &h, bool write){
  static uint16_t widt;
  static uint16_t heig;
  if(write){
    widt = w;
    heig = h;
  }
  else{
    w = widt;
    h = heig;
  }
}
