# epd42_library
### Library extension from the work of [Ben Krasnow (Applied Science)](https://benkrasnow.blogspot.com/2017/10/fast-partial-refresh-on-42-e-paper.html#post-body-2287140971625761519:~:text=Google%20Drive%20link%20with%20Arduino%20firmware,used%20in%20this%20project%3A%20https%3A%2F%2Fdrive.google.com%2Fopen%3Fid%3D0B4YXWiqYWB99UmRYQi1qdXJIVFk).
This extension enables a more cautious use of Direct Updates, while preserving a good contrast, and allows image gray shading (with a default level of gray shades of 8).\
It also enables the use of the ESP32 as a compatible board, but be careful if using the PSRAM or the SD card. The buses are shared...\
To use the ESP32 board, in the file "epdif.h" comment out #define AVR_ARCH. In "SPI_adapter.h" you will find the SPI pin definitions.

See my [website article]() for a more in-depth explanation of the theory behind the operation or to see the results of my analysis, or [contact me](https://deeptronix.wordpress.com/contact/) if you get stuck somewhere while using the library.
