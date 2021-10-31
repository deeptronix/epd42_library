Load files named "drake__.bmp" in your SD card (no subfolders).
Load Arduino sketch to your Teensy, and enjoy.

If you want to use your own image sequences, they need to fulfill these requirements.
The image sequence:
- must be 200 x 150 in size; this is to speed up image loading from SD card, which would otherwise slow down the animation
- must be 24-bit Windows BMP format
- has the same filename as the one specified in the Arduino sketch
- each SD file name is followed by the sequence number (and then the .bmp extension)
