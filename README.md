# epd42_library
### Library extension from the work of [Ben Krasnow (Applied Science)](https://benkrasnow.blogspot.com/2017/10/fast-partial-refresh-on-42-e-paper.html#post-body-2287140971625761519:~:text=Google%20Drive%20link%20with%20Arduino%20firmware,used%20in%20this%20project%3A%20https%3A%2F%2Fdrive.google.com%2Fopen%3Fid%3D0B4YXWiqYWB99UmRYQi1qdXJIVFk).
This extension enables a more cautious use of Direct Updates, while preserving a reasonable contrast, and allows image gray shading (with 8 levels of gray shades).\
It also enables the use of the ESP32 as a compatible board, but be careful if using the PSRAM or the SD card; the buses are shared...\
To use the ESP32 board, in the file "epdif.h" comment out #define AVR_ARCH (which is default for Teensy boards). In that same file you will find the SPI pin definitions for both boards.

#### See my [website article](https://deeptronix.wordpress.com/2021/04/17/video-and-gray-shades-on-epd/) on the subject for a more in-depth explanation of the theory behind the operation, or to see the results of my analysis.
