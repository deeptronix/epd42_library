
#define _mod_bitwise(x, mod)   (x & (mod - 1))

void gray256To8bits(uint8_t *img, uint16_t img_width, uint16_t img_height,   // image width MUST be a multiple of 8.
                                      uint8_t *out_bf, uint8_t size_factor){
  if((size_factor & (size_factor - 1)) != 0)  return;  // halt if size_factor is not a POWER of 2
  uint8_t source, destination = 0;
  uint16_t ind_byte = 0;
  for(uint16_t row = 0; row < (img_height * size_factor); row++){
    for(uint16_t col = 0; col < (img_width * size_factor); col++){
      
      if(!(col % 8)  &&  (col > 0  ||  row > 0)){
        out_bf[ind_byte++] = destination;
        destination = 0;
      }
      source = img[(col/size_factor) + (row/size_factor) * img_width];
      destination |= ((source & 0x80) >> _mod_bitwise(col, 8));
    }
  }
}
