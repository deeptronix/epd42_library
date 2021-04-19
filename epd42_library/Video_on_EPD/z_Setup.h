
#define _max(val_a, val_b)  ((val_a > val_b)?  val_a : val_b)

// EPD paramters:
const uint16_t ep_width = 400, ep_height = 300;
const uint8_t STARTUP_CLEAR_CY = 2; // needed to clear the previous image, which might have forced pixels

// BITMAP parameters:
#define IMG_SIZE_FACTOR 2   // images loaded into the SD card MUST BE half the EPD size (so 200x150); this is to increase loading speed, since it's the biggest bottleneck
const uint16_t img_width = ep_width / IMG_SIZE_FACTOR, img_height = ep_height / IMG_SIZE_FACTOR;
#define BUFFPIXEL 80


// Animation parameters:
const String filename_prefix = "drake";  // this must be directly followed by the sequence position number. Warning: filename cannot exceed 8 chars!
const uint8_t frame_number_start = 1;
const uint16_t total_frames = 24;
const uint16_t ANIMATION_CYCLES = 2;

// Full display deghosting refresh parameters:
// ONLY IF USING DisplayFrameQuickAndHealthy the deghosting cycle count can be this low:
const uint8_t DEGHOST_CYCLES = ((_max(ANIMATION_CYCLES, 2) * total_frames) / 15);
// Default deghosting cycles:
//const uint8_t DEGHOST_CYCLES = 2 * ANIMATION_CYCLES * (total_frames / 20) + 1;

// Execution flow parameters:
#define SLEEP_TIMEOUT_sec 10


// EOF
