/*
  Fennec - SI4732 all-band (AM / FM / SW) receiver on ESP32-S3
  CiferTech  |  MIT License  |  v1.0.0

  Arduino IDE
  -----------
  1. Boards Manager: install "esp32" by Espressif Systems
  2. Tools -> Board -> ESP32S3 Dev Module
  3. Sketch -> Include Library -> Manage Libraries, install:
       - "PU2CLR SI4735"          (Ricardo Caratti)
       - "TFT_eSPI"               (Bodmer)
       - "XPT2046_Touchscreen"    (Paul Stoffregen)
  4. Copy docs/TFT_eSPI_User_Setup.h over:
       Arduino/libraries/TFT_eSPI/User_Setup.h
  5. Open this folder (Fennec.ino) and click Upload

  Pins are in Pins.h. TFT SPI pins must match that User_Setup.h.

  Buttons (GPIO 6 UP / 5 OK / 4 DOWN):
    Menu & tools: UP/DOWN move the highlight, OK selects, hold OK back.
    Hold UP+DOWN (~0.5 s) returns to the menu from any face.
    Radio: UP/DOWN tune (hold = volume), OK band (hold = seek).
*/

#include "App.h"

fennec::App app;

void setup() {
  app.begin();
}

void loop() {
  app.loop();
}
