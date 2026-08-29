#include "DisplayUI.h"

#include <string.h>
#include <Preferences.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "Config.h"
#include "Pins.h"

namespace fennec {

namespace theme {

constexpr uint16_t bg = 0x0841;
constexpr uint16_t sand = 0xE651;
constexpr uint16_t line = 0xCD2D;
constexpr uint16_t muted = 0x8BCA;
constexpr uint16_t dim = 0x3984;

}

const unsigned char kIconAbout[] PROGMEM = {
    0x07, 0xe0, 0x1f, 0xf8, 0x3f, 0xfc, 0x7e, 0x7c, 0x7c, 0x3c, 0x38, 0x3c, 0x00, 0x7c, 0x03, 0xf8,
    0x07, 0xf0, 0x07, 0xc0, 0x03, 0x80, 0x00, 0x00, 0x03, 0x80, 0x07, 0xc0, 0x07, 0xc0, 0x03, 0x80};

const unsigned char kIconSetting[] PROGMEM = {
    0x03, 0xc0, 0x13, 0xc8, 0x3b, 0xdc, 0x7f, 0xfe, 0x3f, 0xfc, 0x1c, 0x38, 0xf8, 0x1f, 0xf8, 0x1f,
    0xf8, 0x1f, 0xf8, 0x1f, 0x1c, 0x38, 0x3f, 0xfc, 0x7f, 0xfe, 0x3b, 0xdc, 0x13, 0xc8, 0x03, 0xc0};

constexpr int16_t kCiferLogoW = 150;
constexpr int16_t kCiferLogoH = 150;
const unsigned char kCiferLogo [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 
  0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x80, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0xf8, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 
  0xe0, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 
  0x00, 0x00, 0x1f, 0xfc, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0xff, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xe1, 0xf0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf3, 0xe0, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 
  0x00, 0x00, 0x1e, 0x07, 0xf8, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 
  0x00, 0x00, 0x7c, 0x00, 0x03, 0xfe, 0x07, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 
  0x00, 0x3f, 0xf8, 0x00, 0x00, 0x3e, 0x00, 0x1f, 0xfe, 0x07, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xf0, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x1f, 0x00, 0x7f, 0xfe, 0x07, 0xff, 0xfc, 0x00, 
  0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 0x03, 0xff, 0xff, 0x80, 0x00, 0x0f, 0x03, 0xff, 0xfe, 0x07, 
  0xff, 0xff, 0x80, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x04, 0x07, 
  0xff, 0xfe, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x1f, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0xe0, 
  0x00, 0x00, 0x1f, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xf8, 0x00, 0x00, 
  0x3f, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x3f, 0xff, 
  0xc0, 0x00, 0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xfe, 0x00, 
  0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x01, 0xff, 0xff, 0xfe, 0x07, 0xff, 
  0xff, 0xff, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x07, 0xff, 0xff, 
  0xfe, 0x07, 0xff, 0xff, 0xff, 0xc0, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 
  0x0f, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
  0x3f, 0xfc, 0x00, 0x1f, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xf0, 0x00, 0xff, 0xe0, 0x00, 
  0x00, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x3f, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xf8, 0x03, 
  0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0x80, 0x7f, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 
  0xff, 0xfc, 0x0f, 0xff, 0xe0, 0x00, 0x00, 0xc0, 0x00, 0x3f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xfe, 
  0x07, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xe0, 0x00, 0x1c, 0xf0, 0x00, 0x1f, 0xff, 0xf8, 0xff, 
  0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xe0, 0x00, 0x7c, 0xf8, 0x00, 0x1f, 
  0xff, 0xf9, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xe0, 0x01, 0xfc, 
  0xfe, 0x00, 0x3f, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xff, 
  0xf8, 0x07, 0xf8, 0x7f, 0x80, 0x7f, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 
  0xff, 0xcf, 0xff, 0xfe, 0x1f, 0xf8, 0x7f, 0xe1, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xfe, 0x07, 
  0xff, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0x7f, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 
  0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 
  0xcf, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xf0, 0x3f, 
  0xff, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xff, 
  0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 
  0xf1, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 
  0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 
  0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xfe, 0x7f, 
  0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 
  0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xfc, 
  0x00, 0x00, 0x7f, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfc, 
  0xff, 0xff, 0xf0, 0x00, 0x00, 0x0f, 0xc1, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 
  0xff, 0xff, 0xfe, 0x7c, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xff, 0xff, 0xff, 0xff, 0xfe, 
  0x07, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xff, 0xff, 
  0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x04, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 
  0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xfe, 
  0x03, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 
  0xfe, 0x07, 0xf0, 0x00, 0x07, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 
  0xff, 0xff, 0xff, 0xfe, 0x07, 0xe0, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xc0, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xc0, 0x00, 0x00, 0x03, 0xfe, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0x80, 0x00, 
  0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xfe, 
  0x07, 0x80, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
  0xff, 0xff, 0xfe, 0x07, 0x80, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0x80, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0x80, 0x00, 0x00, 0x00, 0x7e, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0x80, 0x00, 0x00, 
  0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 
  0xc0, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
  0xff, 0xfe, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xc0, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xe0, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xe0, 0x00, 0x00, 0x00, 
  0xfe, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x7f, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xf0, 
  0x00, 0x00, 0x01, 0xfc, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x7f, 0xff, 0xc0, 0x7f, 
  0xfe, 0x07, 0xf8, 0x00, 0x00, 0x01, 0xfc, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc1, 0xfe, 0x7f, 
  0xff, 0x00, 0x3f, 0xfe, 0x07, 0xf8, 0x00, 0x00, 0x03, 0xfc, 0xff, 0x07, 0x80, 0x00, 0x00, 0x0f, 
  0xff, 0xfe, 0x7f, 0xfe, 0x0e, 0x1f, 0xfe, 0x07, 0xfc, 0x00, 0x00, 0x03, 0xfc, 0xff, 0xff, 0xf0, 
  0x00, 0x00, 0x3f, 0xff, 0xfe, 0x7f, 0xfc, 0x1f, 0x0f, 0xfe, 0x07, 0xfe, 0x00, 0x00, 0x07, 0xfc, 
  0xff, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0xfe, 0x7f, 0xfc, 0x7f, 0xc7, 0xfe, 0x07, 0xff, 0x00, 
  0x00, 0x07, 0xf9, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0x3f, 0xfc, 0x7f, 0xe7, 0xfe, 
  0x07, 0xff, 0x80, 0x00, 0x0f, 0xf9, 0xff, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xff, 0x3f, 0xf8, 
  0xff, 0xf3, 0xfe, 0x07, 0xff, 0xc0, 0x00, 0x1f, 0xf9, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 
  0xff, 0x3f, 0xf8, 0xff, 0xf1, 0xfe, 0x07, 0xff, 0xe0, 0x00, 0x3f, 0xf3, 0xff, 0xff, 0xff, 0xe0, 
  0x0f, 0xff, 0xff, 0xff, 0x9f, 0xfc, 0xff, 0xf8, 0xfe, 0x07, 0xff, 0xf8, 0x00, 0xff, 0xf3, 0xff, 
  0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0x9f, 0xfc, 0xff, 0xfc, 0x7e, 0x07, 0xff, 0xfc, 0x03, 
  0xff, 0xf3, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xcf, 0xfc, 0x7f, 0xfe, 0x3e, 0x07, 
  0xff, 0xff, 0xbf, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0xcf, 0xfc, 0x7f, 
  0xff, 0x1e, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xfe, 0x3f, 0xf8, 0x3f, 0xf8, 0xff, 0xff, 
  0xe7, 0xfe, 0x3f, 0xff, 0x9e, 0x07, 0xf0, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xfc, 0x0f, 0xf8, 0x3f, 
  0xe0, 0x3f, 0xff, 0xe3, 0xfe, 0x1f, 0xff, 0x8e, 0x07, 0xc0, 0x3f, 0xff, 0xff, 0xcf, 0xff, 0xf0, 
  0x07, 0xf8, 0x3f, 0x80, 0x0f, 0xff, 0xf3, 0xff, 0x1f, 0xff, 0xc6, 0x07, 0x82, 0x1f, 0xff, 0xff, 
  0x9f, 0xff, 0xe0, 0x01, 0xfc, 0x3f, 0x00, 0x07, 0xff, 0xf9, 0xff, 0x8f, 0xff, 0xe2, 0x07, 0x0f, 
  0x0f, 0xff, 0xfe, 0x1f, 0xff, 0xe0, 0x00, 0x7c, 0x3c, 0x00, 0x07, 0xff, 0xe0, 0x7f, 0x87, 0xff, 
  0xf0, 0x06, 0x3f, 0xc7, 0xff, 0xfc, 0x0f, 0xff, 0xe0, 0x00, 0x1c, 0x30, 0x00, 0x07, 0xff, 0x80, 
  0x1f, 0xc3, 0xff, 0xf8, 0x0c, 0x7f, 0xe3, 0xff, 0xf0, 0x03, 0xff, 0xe0, 0x00, 0x08, 0x00, 0x00, 
  0x07, 0xff, 0x00, 0x03, 0xe1, 0xff, 0xf8, 0x00, 0xff, 0xf1, 0xff, 0xc0, 0x01, 0xff, 0xe0, 0x00, 
  0x00, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 0xf0, 0xff, 0xfc, 0x01, 0xff, 0xf8, 0xfe, 0x00, 0x00, 
  0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x00, 0x00, 0x78, 0x7f, 0xfe, 0x03, 0xff, 0xf8, 
  0xfc, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfe, 0x00, 0x00, 0x1c, 0x7f, 0xff, 
  0x0f, 0xff, 0xfc, 0xf8, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 
  0x1e, 0x3f, 0xff, 0xdf, 0xff, 0xf8, 0xf0, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 
  0xfc, 0x00, 0x00, 0x0f, 0x1f, 0xff, 0xff, 0xff, 0xf1, 0xe0, 0x00, 0x00, 0x7f, 0xff, 0x80, 0x00, 
  0x00, 0x03, 0xff, 0xfc, 0x00, 0x00, 0x07, 0x8f, 0xff, 0xff, 0xff, 0xe1, 0xc0, 0x00, 0x00, 0x3f, 
  0xff, 0xc0, 0x00, 0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x07, 0xc7, 0xff, 0xff, 0xff, 0xc3, 0xc0, 
  0x00, 0x00, 0x1f, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xe3, 0xff, 0xff, 
  0xff, 0x87, 0x80, 0x00, 0x00, 0x0f, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xe0, 0x00, 0x00, 0x03, 
  0xf1, 0xff, 0xff, 0xff, 0x1f, 0x80, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x7f, 0xff, 0xc0, 
  0x00, 0x00, 0x01, 0xf8, 0xff, 0xff, 0xfc, 0x3f, 0x80, 0x00, 0x00, 0x01, 0xff, 0xfe, 0x00, 0x00, 
  0x1f, 0xff, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x7f, 0xff, 0xf8, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 
  0xf8, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x3f, 0xff, 0xf0, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x7f, 0xff, 0xf0, 
  0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 
  0xff, 0xff, 0xfc, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x61, 0xff, 0xff, 0xfe, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xe0, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 
  0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x7f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xcf, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x87, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfe, 0x01, 0xff, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 
  0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 
  0xff, 0xf8, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00
};


namespace {

TFT_eSPI tft;
TFT_eSprite huntSpr(&tft);
constexpr int16_t kHuntSprW = 222;
constexpr int16_t kHuntSprH = 158;
constexpr int16_t kHuntPlotX = 20;
constexpr int16_t kHuntPlotTop = 8;
constexpr int16_t kHuntPlotBot = 150;
SPIClass touchSPI = SPIClass(HSPI);
XPT2046_Touchscreen ts(PIN_TOUCH_CS);

constexpr uint8_t kFontBig = 6;
constexpr uint8_t kBlChannel = 0;
constexpr uint32_t kBlHz = 5000;

void backlightAttach() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(PIN_TFT_BL, kBlHz, 8);
#else
  ledcSetup(kBlChannel, kBlHz, 8);
  ledcAttachPin(PIN_TFT_BL, kBlChannel);
#endif
}

void backlightWrite(uint8_t duty) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(PIN_TFT_BL, duty);
#else
  ledcWrite(kBlChannel, duty);
#endif
}

struct TouchCal {
  int16_t xMin;
  int16_t xMax;
  int16_t yMin;
  int16_t yMax;
  uint8_t flags;
};

TouchCal gCal;

void loadTouchCal() {
  gCal.xMin = FENNEC_TOUCH_CAL_0;
  gCal.xMax = FENNEC_TOUCH_CAL_1;
  gCal.yMin = FENNEC_TOUCH_CAL_2;
  gCal.yMax = FENNEC_TOUCH_CAL_3;
  gCal.flags = FENNEC_TOUCH_CAL_4;
  Preferences p;
  if (!p.begin("wbtouch", true)) {
    return;
  }
  gCal.xMin = p.getShort("xmin", gCal.xMin);
  gCal.xMax = p.getShort("xmax", gCal.xMax);
  gCal.yMin = p.getShort("ymin", gCal.yMin);
  gCal.yMax = p.getShort("ymax", gCal.yMax);
  gCal.flags = p.getUChar("flags", gCal.flags);
  p.end();
}

void saveTouchCal() {
  Preferences p;
  if (!p.begin("wbtouch", false)) {
    return;
  }
  p.putShort("xmin", gCal.xMin);
  p.putShort("xmax", gCal.xMax);
  p.putShort("ymin", gCal.yMin);
  p.putShort("ymax", gCal.yMax);
  p.putUChar("flags", gCal.flags);
  p.end();
}

struct Rect {
  int16_t x, y, w, h;
};

const Rect kTabFm = {8, 28, 50, 20};
const Rect kTabMw = {66, 28, 50, 20};
const Rect kTabSw = {124, 28, 50, 20};
const Rect kTabMore = {182, 28, 50, 20};

const Rect kMenuTile[5] = {{12, 64, 216, 36},  {12, 110, 216, 36}, {12, 156, 216, 36},
                           {12, 202, 216, 36}, {12, 248, 216, 36}};
const Rect kMenuAbout = {12, 7, 32, 20};
const Rect kMenuSettings = {196, 7, 32, 20};

const Rect kHuntPeak = {178, 7, 54, 20};
const Rect kHuntPlot = {8, 36, 224, 160};
const Rect kCwPitchDn = {8, 218, 54, 32};
const Rect kCwPitchVal = {64, 218, 112, 32};
const Rect kCwPitchUp = {178, 218, 54, 32};
const Rect kCwBfoDn = {8, 252, 54, 32};
const Rect kCwBfoVal = {64, 252, 112, 32};
const Rect kCwBfoUp = {178, 252, 54, 32};
const Rect kCwVolDn = {8, 286, 54, 32};
const Rect kCwVolVal = {64, 286, 112, 32};
const Rect kCwVolUp = {178, 286, 54, 32};
const Rect kCwCopy = {8, 36, 224, 148};
const Rect kCwSrc = {8, 188, 56, 26};
const Rect kCwHud = {66, 188, 166, 26};
const Rect kLogFreqDn = {8, 222, 54, 28};
const Rect kLogFreqVal = {64, 222, 112, 28};
const Rect kLogFreqUp = {178, 222, 54, 28};
const Rect kLogArm = {64, 292, 54, 26};
const Rect kLogPreset[5] = {
    {8, 258, 44, 26}, {52, 258, 44, 26}, {96, 258, 44, 26}, {140, 258, 44, 26}, {184, 258, 48, 26}};
const Rect kLogThrDn = {8, 292, 54, 26};
const Rect kLogClr = {120, 292, 54, 26};
const Rect kLogThrUp = {176, 292, 56, 26};
const Rect kLogList = {8, 36, 224, 160};

const Rect kSstvBack = {8, 7, 54, 20};
const Rect kSstvUsb = {178, 7, 54, 20};
const Rect kSstvSrc = {8, 216, 56, 26};
const Rect kSstvMode = {66, 216, 88, 26};
const Rect kSstvPic = {8, 36, kSstvOutW, kSstvOutH};
const Rect kSstvCall[4] = {{8, 246, 56, 28}, {64, 246, 56, 28}, {120, 246, 56, 28}, {176, 246, 56, 28}};
const Rect kSstvVolDn = {8, 278, 43, 34};
const Rect kSstvBfoDn = {53, 278, 43, 34};
const Rect kSstvReset = {98, 278, 44, 34};
const Rect kSstvBfoUp = {144, 278, 43, 34};
const Rect kSstvVolUp = {189, 278, 43, 34};
const Rect kHintBar = {8, 300, 224, 20};

const Rect kBtnTuneDn = {8, 266, 68, 22};
const Rect kBtnSeek = {86, 266, 68, 22};
const Rect kBtnTuneUp = {164, 266, 68, 22};
const Rect kBtnVolDn = {8, 294, 68, 22};
const Rect kBtnMute = {86, 294, 68, 22};
const Rect kBtnVolUp = {164, 294, 68, 22};

const Rect kBandCompact[3] = {{8, 58, 68, 18}, {86, 58, 68, 18}, {164, 58, 68, 18}};

bool inside(int16_t x, int16_t y, const Rect& r) {
  return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

uint8_t slotOfIndex(BandKind kind, uint8_t index) {
  uint8_t slot = 0;
  for (uint8_t i = 0; i < kBandCount; ++i) {
    if (kBands[i].kind != kind) {
      continue;
    }
    if (i == index) {
      return slot;
    }
    ++slot;
  }
  return 0;
}

uint8_t compactBandIndex(BandKind kind, uint8_t currentIndex, uint8_t pos) {
  const uint8_t n = slotCountOf(kind);
  if (n == 0) {
    return currentIndex;
  }
  const uint8_t cur = slotOfIndex(kind, currentIndex);
  uint8_t slot = cur;
  if (pos == 0) {
    slot = static_cast<uint8_t>((cur + n - 1) % n);
  } else if (pos == 2) {
    slot = static_cast<uint8_t>((cur + 1) % n);
  }
  return indexFromSlot(kind, slot);
}

void touchBusBegin() {
  touchSPI.begin(PIN_TOUCH_CLK, PIN_TOUCH_DO, PIN_TOUCH_DIN, PIN_TOUCH_CS);
  pinMode(PIN_TOUCH_CS, OUTPUT);
  digitalWrite(PIN_TOUCH_CS, HIGH);
}

bool readTouch(int16_t* x, int16_t* y) {
  touchBusBegin();
  const TS_Point p = ts.getPoint();

#if FENNEC_TOUCH_DEBUG
  static uint32_t lastDbg = 0;
  if (p.z > 30 && (millis() - lastDbg > 200)) {
    lastDbg = millis();
    Serial.printf("[touch] raw x=%d y=%d z=%d\n", p.x, p.y, p.z);
  }
#endif

  if (p.z < FENNEC_TOUCH_Z_MIN) {
    return false;
  }

  int32_t rx = p.x;
  int32_t ry = p.y;
  if (gCal.flags & 1) {
    const int32_t t = rx;
    rx = ry;
    ry = t;
  }

  const int16_t maxX = static_cast<int16_t>(tft.width() - 1);
  const int16_t maxY = static_cast<int16_t>(tft.height() - 1);
  int16_t sx = static_cast<int16_t>(map(rx, gCal.xMin, gCal.xMax, 0, maxX));
  int16_t sy = static_cast<int16_t>(map(ry, gCal.yMin, gCal.yMax, 0, maxY));
  if (gCal.flags & 2) {
    sx = static_cast<int16_t>(maxX - sx);
  }
  if (gCal.flags & 4) {
    sy = static_cast<int16_t>(maxY - sy);
  }
  *x = constrain(sx, 0, maxX);
  *y = constrain(sy, 0, maxY);
  return true;
}

void useBitmapFont(uint8_t font) {
  tft.setFreeFont(nullptr);
  tft.setTextFont(font);
}

void drawCornerTicks(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t arm = 5,
                     uint16_t color = theme::line) {
  tft.drawFastHLine(x0, y0, arm, color);
  tft.drawFastVLine(x0, y0, arm, color);
  tft.drawFastHLine(static_cast<int16_t>(x1 - arm + 1), y0, arm, color);
  tft.drawFastVLine(x1, y0, arm, color);
  tft.drawFastHLine(x0, y1, arm, color);
  tft.drawFastVLine(x0, static_cast<int16_t>(y1 - arm + 1), arm, color);
  tft.drawFastHLine(static_cast<int16_t>(x1 - arm + 1), y1, arm, color);
  tft.drawFastVLine(x1, static_cast<int16_t>(y1 - arm + 1), arm, color);
}

void drawBrackets(int16_t y0, int16_t y1, int16_t arm = 5) {
  drawCornerTicks(2, y0, 237, y1, arm);
}

template <typename G>
int16_t spacedWidth(G& g, const char* s, uint8_t gap, uint8_t font) {
  int16_t w = 0;
  for (const char* p = s; *p; ++p) {
    const char c[2] = {*p, 0};
    w += g.textWidth(c, font) + gap;
  }
  return s[0] ? static_cast<int16_t>(w - gap) : 0;
}

template <typename G>
void spacedAtLeft(G& g, const char* s, int16_t x, int16_t cy, uint8_t gap, uint8_t font, uint16_t fg,
                  uint16_t bgc) {
  g.setFreeFont(nullptr);
  g.setTextFont(font);
  g.setTextDatum(TL_DATUM);
  g.setTextColor(fg, bgc);
  const int16_t y = cy - g.fontHeight(font) / 2;
  for (const char* p = s; *p; ++p) {
    const char c[2] = {*p, 0};
    g.drawString(c, x, y, font);
    x += g.textWidth(c, font) + gap;
  }
}

template <typename G>
void spacedAt(G& g, const char* s, int16_t cx, int16_t cy, uint8_t gap, uint8_t font, uint16_t fg,
              uint16_t bgc) {
  g.setFreeFont(nullptr);
  g.setTextFont(font);
  spacedAtLeft(g, s, static_cast<int16_t>(cx - spacedWidth(g, s, gap, font) / 2), cy, gap, font, fg, bgc);
}

void drawSpaced(const char* s, int16_t cx, int16_t cy, uint8_t gap, uint8_t font, uint16_t fg) {
  spacedAt(tft, s, cx, cy, gap, font, fg, theme::bg);
}

void drawSpacedLeft(const char* s, int16_t x, int16_t cy, uint8_t gap, uint8_t font, uint16_t fg) {
  spacedAtLeft(tft, s, x, cy, gap, font, fg, theme::bg);
}

void drawSpacedRight(const char* s, int16_t right, int16_t cy, uint8_t gap, uint8_t font, uint16_t fg) {
  useBitmapFont(font);
  const int16_t w = spacedWidth(tft, s, gap, font);
  drawSpacedLeft(s, static_cast<int16_t>(right - w), cy, gap, font, fg);
}

void drawSignalGlyph(int16_t x, int16_t baseline, uint8_t rssi, bool live) {
  for (uint8_t i = 0; i < 4; ++i) {
    const int16_t h = static_cast<int16_t>(2 + i * 2);
    const bool on = live && rssi >= static_cast<uint8_t>(8 + i * 16);
    tft.fillRect(static_cast<int16_t>(x + i * 4), static_cast<int16_t>(baseline - h), 2, h,
                 on ? theme::sand : theme::dim);
  }
}

void drawCwValue(const Rect& r, const char* s, uint16_t fg) {
  tft.fillRect(static_cast<int16_t>(r.x + 1), static_cast<int16_t>(r.y + 1),
               static_cast<int16_t>(r.w - 2), static_cast<int16_t>(r.h - 2), theme::bg);
  drawSpaced(s, static_cast<int16_t>(r.x + r.w / 2), static_cast<int16_t>(r.y + r.h / 2), 2, 2, fg);
}

void drawCwStatus(const Rect& r, const char* left, uint16_t leftFg, const char* right) {
  tft.fillRect(r.x, r.y, r.w, r.h, theme::bg);
  tft.drawRect(r.x, r.y, r.w, r.h, theme::dim);
  const int16_t cy = static_cast<int16_t>(r.y + r.h / 2 + 1);
  drawSpacedLeft(left, static_cast<int16_t>(r.x + 8), cy, 1, 1, leftFg);
  drawSpacedRight(right, static_cast<int16_t>(r.x + r.w - 8), cy, 1, 1, theme::muted);
}

void drawCwCopy(const char* t, int16_t x, int16_t y, int16_t w, int16_t h) {
  tft.fillRect(x, y, w, h, theme::bg);
  useBitmapFont(2);
  tft.setTextWrap(false);
  tft.setTextDatum(TL_DATUM);
  if (!t[0]) {
    drawSpacedLeft("LISTENING", x + 2, y + 16, 2, 2, theme::muted);
    return;
  }

  const int16_t rowH = static_cast<int16_t>(tft.fontHeight(2) + 2);
  const int maxRows = h / rowH;
  if (maxRows <= 0) {
    return;
  }

  struct Seg {
    uint16_t off;
    uint8_t n;
  };
  Seg segs[24];
  uint8_t nSeg = 0;
  const uint16_t n = static_cast<uint16_t>(strlen(t));
  uint16_t i = 0;
  while (i < n && nSeg < 24) {
    while (i < n && t[i] == ' ') {
      ++i;
    }
    if (i >= n) {
      break;
    }
    const uint16_t start = i;
    int16_t lw = 0;
    while (i < n) {
      const char ch[2] = {t[i], 0};
      const int16_t cw = tft.textWidth(ch, 2);
      if (lw > 0 && lw + cw > w) {
        break;
      }
      lw += cw;
      ++i;
    }
    if (i == start) {
      ++i;
    }
    segs[nSeg].off = start;
    segs[nSeg].n = static_cast<uint8_t>(i - start);
    ++nSeg;
  }

  const uint8_t first = nSeg > maxRows ? static_cast<uint8_t>(nSeg - maxRows) : 0;
  tft.setTextColor(theme::sand, theme::bg);
  int16_t py = y;
  char buf[40];
  for (uint8_t s = first; s < nSeg; ++s) {
    uint8_t L = segs[s].n;
    if (L >= sizeof(buf)) {
      L = sizeof(buf) - 1;
    }
    memcpy(buf, t + segs[s].off, L);
    buf[L] = 0;
    tft.drawString(buf, x, py, 2);
    py = static_cast<int16_t>(py + rowH);
  }
}

void drawIconKey(const Rect& r, const unsigned char* bmp, bool lit, bool alignRight = false) {
  tft.fillRect(r.x, r.y, r.w, r.h, theme::bg);
  constexpr int16_t kPad = 3;
  const int16_t ix = alignRight ? static_cast<int16_t>(r.x + r.w - 16 - kPad)
                                : static_cast<int16_t>(r.x + kPad);
  const int16_t iy = static_cast<int16_t>(r.y + (r.h - 16) / 2);
  const uint16_t fg = lit ? theme::sand : theme::line;
  drawCornerTicks(static_cast<int16_t>(ix - kPad), static_cast<int16_t>(iy - kPad),
                  static_cast<int16_t>(ix + 16 + kPad - 1), static_cast<int16_t>(iy + 16 + kPad - 1), 4,
                  fg);
  tft.drawBitmap(ix, iy, bmp, 16, 16, fg);
}

void drawInfoRow(int16_t y, const char* label, const char* value) {
  drawSpacedLeft(label, 16, y, 1, 1, theme::muted);
  drawSpacedLeft(value, 80, y, 1, 1, theme::sand);
}

void drawOutlineKey(const Rect& r, const char* label, bool lit) {
  tft.fillRect(r.x, r.y, r.w, r.h, theme::bg);
  tft.drawRect(r.x, r.y, r.w, r.h, lit ? theme::sand : theme::dim);
  if (lit) {
    tft.drawFastHLine(r.x + 4, r.y + r.h - 4, r.w - 8, theme::sand);
  }
  const uint16_t fg = lit ? theme::sand : theme::muted;
  if (spacedWidth(tft, label, 1, 1) <= r.w - 6) {
    drawSpaced(label, r.x + r.w / 2, r.y + r.h / 2 + 1, 1, 1, fg);
  } else {
    useBitmapFont(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(fg, theme::bg);
    tft.drawString(label, r.x + r.w / 2, r.y + r.h / 2, 1);
  }
}

void drawTab(const Rect& r, const char* label, bool on) {
  tft.fillRect(r.x, r.y, r.w, r.h, on ? theme::dim : theme::bg);
  tft.drawRect(r.x, r.y, r.w, r.h, on ? theme::sand : theme::line);
  const uint16_t fg = on ? theme::sand : theme::line;
  drawSpaced(label, static_cast<int16_t>(r.x + r.w / 2), static_cast<int16_t>(r.y + r.h / 2), 2, 1, fg);
}

bool isSstvHit(TouchEvent::Type t) {
  return t == TouchEvent::SstvBack || t == TouchEvent::SstvUsb || t == TouchEvent::SstvSrc ||
         t == TouchEvent::SstvCycleMode || t == TouchEvent::SstvCall ||
         t == TouchEvent::SstvBfoDown || t == TouchEvent::SstvBfoUp || t == TouchEvent::SstvReset;
}

uint8_t sstvSlotFromFreq(uint16_t khz) {
  static const uint16_t kCall[4] = {3733, 7171, 14230, 21340};
  for (uint8_t i = 0; i < 4; ++i) {
    if (khz + 2 >= kCall[i] && khz <= kCall[i] + 2) {
      return i;
    }
  }
  return 255;
}

void drawCross(int16_t x, int16_t y, uint16_t color) {
  tft.drawLine(x - 12, y, x + 12, y, color);
  tft.drawLine(x, y - 12, x, y + 12, color);
}

void waitTouchUp() {
  uint32_t idle = millis();
  while (millis() - idle < 40000) {
    touchBusBegin();
    if (ts.getPoint().z < 80) {
      delay(150);
      return;
    }
    delay(10);
    yield();
  }
}

bool waitCalSample(int16_t* x, int16_t* y) {
  waitTouchUp();
  uint32_t start = millis();
  while (true) {
    touchBusBegin();
    const TS_Point p = ts.getPoint();
    if (p.z >= FENNEC_TOUCH_Z_MIN) {
      break;
    }
    if (millis() - start > 30000) {
      return false;
    }
    delay(10);
    yield();
  }
  delay(60);
  int32_t ax = 0;
  int32_t ay = 0;
  int n = 0;
  for (int i = 0; i < 16; ++i) {
    touchBusBegin();
    const TS_Point p = ts.getPoint();
    if (p.z >= FENNEC_TOUCH_Z_MIN) {
      ax += p.x;
      ay += p.y;
      ++n;
    }
    delay(12);
  }
  waitTouchUp();
  if (n < 6) {
    return false;
  }
  *x = static_cast<int16_t>(ax / n);
  *y = static_cast<int16_t>(ay / n);
  return true;
}

bool runTouchCalibration() {
  const int16_t w = static_cast<int16_t>(tft.width());
  const int16_t h = static_cast<int16_t>(tft.height());
  const int16_t m = 22;
  const int16_t tx[4] = {m, static_cast<int16_t>(w - 1 - m), static_cast<int16_t>(w - 1 - m), m};
  const int16_t ty[4] = {m, m, static_cast<int16_t>(h - 1 - m), static_cast<int16_t>(h - 1 - m)};
  int16_t rx[4];
  int16_t ry[4];

  tft.fillScreen(theme::bg);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(theme::sand, theme::bg);
  tft.drawString("TOUCH CAL", w / 2, h / 2 - 20, 2);
  tft.setTextColor(theme::muted, theme::bg);
  tft.drawString("Tap each cross", w / 2, h / 2 + 10, 2);
  delay(800);

  for (uint8_t i = 0; i < 4; ++i) {
    tft.fillScreen(theme::bg);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(theme::muted, theme::bg);
    tft.drawString("Tap the cross", w / 2, h / 2, 2);
    drawCross(tx[i], ty[i], theme::sand);
    if (!waitCalSample(&rx[i], &ry[i])) {
      Serial.println(F("[touch] calibration cancelled"));
      return false;
    }
    drawCross(tx[i], ty[i], theme::line);
    delay(200);
  }

  const int adx = abs(rx[1] - rx[0]);
  const int ady = abs(ry[1] - ry[0]);
  uint8_t flags = 0;
  int16_t ax[4];
  int16_t ay[4];
  if (ady > adx) {
    flags = 1;
    for (uint8_t i = 0; i < 4; ++i) {
      ax[i] = ry[i];
      ay[i] = rx[i];
    }
  } else {
    for (uint8_t i = 0; i < 4; ++i) {
      ax[i] = rx[i];
      ay[i] = ry[i];
    }
  }

  gCal.xMin = static_cast<int16_t>((ax[0] + ax[3]) / 2);
  gCal.xMax = static_cast<int16_t>((ax[1] + ax[2]) / 2);
  gCal.yMin = static_cast<int16_t>((ay[0] + ay[1]) / 2);
  gCal.yMax = static_cast<int16_t>((ay[3] + ay[2]) / 2);
  gCal.flags = flags;
  saveTouchCal();

  Serial.printf("[touch] cal xmin=%d xmax=%d ymin=%d ymax=%d flags=%u\n", gCal.xMin, gCal.xMax,
                gCal.yMin, gCal.yMax, gCal.flags);

  tft.fillScreen(theme::bg);
  tft.setTextColor(theme::sand, theme::bg);
  tft.drawString("Saved", w / 2, h / 2, 4);
  delay(600);
  return true;
}

void drawToolTitle(const char* title) {
  constexpr uint8_t kGap = 3;
  constexpr uint8_t kFont = 2;
  constexpr int16_t kPad = 6;
  const int16_t w = spacedWidth(tft, title, kGap, kFont);
  const int16_t x0 = static_cast<int16_t>(120 - w / 2 - kPad);
  const int16_t x1 = static_cast<int16_t>(x0 + w + 2 * kPad - 1);
  const int16_t y0 = 7;
  const int16_t y1 = 26;
  const int16_t arm = 4;
  tft.drawFastHLine(x0, y0, arm, theme::line);
  tft.drawFastVLine(x0, y0, arm, theme::line);
  tft.drawFastHLine(static_cast<int16_t>(x1 - arm + 1), y0, arm, theme::line);
  tft.drawFastVLine(x1, y0, arm, theme::line);
  tft.drawFastHLine(x0, y1, arm, theme::line);
  tft.drawFastVLine(x0, static_cast<int16_t>(y1 - arm + 1), arm, theme::line);
  tft.drawFastHLine(static_cast<int16_t>(x1 - arm + 1), y1, arm, theme::line);
  tft.drawFastVLine(x1, static_cast<int16_t>(y1 - arm + 1), arm, theme::line);
  drawSpaced(title, 120, 17, kGap, kFont, theme::sand);
}

void dashHLine(TFT_eSprite& s, int16_t x, int16_t y, int16_t w, uint16_t color) {
  for (int16_t i = 0; i < w; i += 5) {
    const int16_t n = static_cast<int16_t>(w - i);
    s.drawFastHLine(x + i, y, n > 3 ? 3 : n, color);
  }
}

int16_t huntPlotY(uint8_t v) {
  if (v > 80) {
    v = 80;
  }
  constexpr int16_t kSpan = kHuntPlotBot - kHuntPlotTop;
  return static_cast<int16_t>(kHuntPlotBot - (static_cast<int>(v) * kSpan / 80));
}

void drawHuntMeterChrome(int16_t y, const char* label) {
  drawSpacedLeft(label, 10, static_cast<int16_t>(y + 6), 1, 1, theme::muted);
  tft.fillRect(8, static_cast<int16_t>(y + 14), 224, 6, theme::dim);
}

void drawHuntMeterData(int16_t y, uint8_t value, uint8_t scale, int peakMark, const char* rhs,
                       uint16_t rhsColor) {
  tft.fillRect(80, y, 152, 13, theme::bg);
  char num[8];
  snprintf(num, sizeof(num), "%02u", value);
  if (rhs != nullptr && rhs[0] != 0) {
    char combo[20];
    snprintf(combo, sizeof(combo), "%s  %s", num, rhs);
    drawSpacedRight(combo, 230, static_cast<int16_t>(y + 6), 1, 1, rhsColor);
  } else {
    drawSpacedRight(num, 230, static_cast<int16_t>(y + 6), 1, 1, theme::sand);
  }
  constexpr int16_t kBarX = 8;
  constexpr int16_t kBarW = 224;
  constexpr int16_t kBarH = 6;
  const int16_t by = static_cast<int16_t>(y + 14);
  tft.fillRect(kBarX, static_cast<int16_t>(by - 2), kBarW, kBarH + 4, theme::bg);
  tft.fillRect(kBarX, by, kBarW, kBarH, theme::dim);
  uint16_t fill = scale == 0 ? 0 : static_cast<uint16_t>(value) * kBarW / scale;
  if (fill > kBarW) {
    fill = kBarW;
  }
  if (fill > 0) {
    tft.fillRect(kBarX, by, static_cast<int16_t>(fill), kBarH, theme::sand);
  }
  if (peakMark > 0) {
    uint16_t pkx = scale == 0 ? 0 : static_cast<uint16_t>(peakMark) * kBarW / scale;
    if (pkx >= kBarW) {
      pkx = kBarW - 1;
    }
    tft.drawFastVLine(static_cast<int16_t>(kBarX + pkx), static_cast<int16_t>(by - 2), kBarH + 4,
                      theme::line);
  }
}

const char* sstvModeLabel(SstvMode mode, bool finished) {
  if (finished) {
    return "DONE";
  }
  switch (mode) {
    case SstvMode::MartinM1:
      return "MARTIN 1";
    case SstvMode::MartinM2:
      return "MARTIN 2";
    case SstvMode::ScottieS1:
      return "SCOTTIE 1";
    case SstvMode::ScottieS2:
      return "SCOTTIE 2";
    case SstvMode::Robot36:
      return "ROBOT 36";
    default:
      return "AUTO";
  }
}

const char* sstvLockLabel(SstvMode mode, SstvMode prefer, bool finished) {
  if (finished) {
    return "DONE";
  }
  const SstvMode m = mode != SstvMode::Unknown ? mode : prefer;
  return sstvModeLabel(m, false);
}

void drawSstvCallRow(uint8_t slot, int8_t pressSlot) {
  const Rect& a = kSstvCall[0];
  const int16_t x = a.x;
  const int16_t y = a.y;
  const int16_t w = static_cast<int16_t>(kSstvCall[3].x + kSstvCall[3].w - x);
  const int16_t h = a.h;
  tft.fillRect(x, y, w, h, theme::bg);
  tft.drawRect(x, y, w, h, theme::dim);
  const char* labs[4] = {"80M", "40M", "20M", "15M"};
  for (uint8_t i = 0; i < 4; ++i) {
    const Rect& r = kSstvCall[i];
    if (i > 0) {
      tft.drawFastVLine(r.x, static_cast<int16_t>(y + 4), static_cast<int16_t>(h - 8), theme::dim);
    }
    const bool on = slot == i || pressSlot == static_cast<int8_t>(i);
    drawSpaced(labs[i], static_cast<int16_t>(r.x + r.w / 2), static_cast<int16_t>(r.y + r.h / 2), 1, 1,
               on ? theme::sand : theme::muted);
    if (on) {
      tft.drawFastHLine(static_cast<int16_t>(r.x + 8), static_cast<int16_t>(r.y + r.h - 4), r.w - 16,
                        theme::sand);
    }
  }
}

void drawLogPresetRow(uint8_t slot, int8_t pressSlot) {
  const Rect& a = kLogPreset[0];
  const int16_t x = a.x;
  const int16_t y = a.y;
  const int16_t w = static_cast<int16_t>(kLogPreset[4].x + kLogPreset[4].w - x);
  const int16_t h = a.h;
  tft.fillRect(x, y, w, h, theme::bg);
  tft.drawRect(x, y, w, h, theme::dim);
  const NsPreset* p = NsLogger::presets();
  for (uint8_t i = 0; i < kNsPresetCount; ++i) {
    const Rect& r = kLogPreset[i];
    if (i > 0) {
      tft.drawFastVLine(r.x, static_cast<int16_t>(y + 4), static_cast<int16_t>(h - 8), theme::dim);
    }
    const bool on = slot == i || pressSlot == static_cast<int8_t>(i);
    drawSpaced(p[i].name, static_cast<int16_t>(r.x + r.w / 2), static_cast<int16_t>(r.y + r.h / 2), 1,
               1, on ? theme::sand : theme::muted);
    if (on) {
      tft.drawFastHLine(static_cast<int16_t>(r.x + 6), static_cast<int16_t>(r.y + r.h - 4), r.w - 12,
                        theme::sand);
    }
  }
}
uint8_t sstvHudEpoch = 0;
bool cwUsbShown = true;
bool cwMicShown = false;

struct FocusItem {
  TouchEvent::Type type;
  uint8_t slot;
};

const FocusItem* focusTable(UiScreen s, uint8_t* n) {
  static const FocusItem kMenu[] = {
      {TouchEvent::MorePick, 0}, {TouchEvent::MorePick, 1}, {TouchEvent::MorePick, 2},
      {TouchEvent::MorePick, 3}, {TouchEvent::MorePick, 4},
      {TouchEvent::OpenAbout, 0}, {TouchEvent::OpenSettings, 0},
  };
  static const FocusItem kSet[] = {
      {TouchEvent::SettingsPick, 0},
      {TouchEvent::SettingsPick, 1},
      {TouchEvent::SettingsPick, 2},
      {TouchEvent::SettingsPick, 3},
  };
  static const FocusItem kSstv[] = {
      {TouchEvent::SstvBack, 0},      {TouchEvent::SstvUsb, 0},      {TouchEvent::SstvSrc, 0},
      {TouchEvent::SstvCycleMode, 0}, {TouchEvent::SstvCall, 0},     {TouchEvent::SstvCall, 1},
      {TouchEvent::SstvCall, 2},      {TouchEvent::SstvCall, 3},     {TouchEvent::VolDown, 0},
      {TouchEvent::SstvBfoDown, 0},   {TouchEvent::SstvReset, 0},    {TouchEvent::SstvBfoUp, 0},
      {TouchEvent::VolUp, 0},
  };
  static const FocusItem kCw[] = {
      {TouchEvent::SstvBack, 0},    {TouchEvent::SstvUsb, 0},     {TouchEvent::CwSrc, 0},
      {TouchEvent::CwPitchDown, 0}, {TouchEvent::CwPitchUp, 0},   {TouchEvent::SstvBfoDown, 0},
      {TouchEvent::SstvBfoUp, 0},   {TouchEvent::VolDown, 0},     {TouchEvent::VolUp, 0},
  };
  static const FocusItem kLog[] = {
      {TouchEvent::SstvBack, 0},  {TouchEvent::SstvUsb, 0}, {TouchEvent::TuneDown, 0},
      {TouchEvent::TuneUp, 0},    {TouchEvent::LogPreset, 0}, {TouchEvent::LogPreset, 1},
      {TouchEvent::LogPreset, 2}, {TouchEvent::LogPreset, 3}, {TouchEvent::LogPreset, 4},
      {TouchEvent::LogThrDown, 0},{TouchEvent::LogArm, 0},    {TouchEvent::LogClr, 0},
      {TouchEvent::LogThrUp, 0},
  };
  switch (s) {
    case UiScreen::Menu:
      *n = 7;
      return kMenu;
    case UiScreen::Settings:
      *n = 4;
      return kSet;
    case UiScreen::Sstv:
      *n = 13;
      return kSstv;
    case UiScreen::Cw:
      *n = 9;
      return kCw;
    case UiScreen::Log:
      *n = 13;
      return kLog;
    default:
      *n = 0;
      return nullptr;
  }
}

}

void DisplayUI::setBrightness(uint8_t pct) {
  if (pct < FENNEC_BRIGHTNESS_MIN) {
    pct = FENNEC_BRIGHTNESS_MIN;
  }
  if (pct > 100) {
    pct = 100;
  }
  brightness_ = pct;
  settingsBright_ = pct;
  const uint8_t duty = static_cast<uint8_t>((static_cast<uint16_t>(pct) * 255 + 50) / 100);
  backlightWrite(duty);
}

uint8_t DisplayUI::btnFocusCount() const {
  uint8_t n = 0;
  focusTable(screen_, &n);
  return n;
}

bool DisplayUI::btnLit(TouchEvent::Type t, uint8_t slot) const {
  if (touchDown_ && heldType_ == t) {
    if (t == TouchEvent::MorePick || t == TouchEvent::SettingsPick || t == TouchEvent::SstvCall ||
        t == TouchEvent::LogPreset || t == TouchEvent::SwBand) {
      return sstvPressSlot_ == slot;
    }
    return true;
  }
  uint8_t n = 0;
  const FocusItem* it = focusTable(screen_, &n);
  if (it == nullptr || btnFocus_ >= n) {
    return false;
  }
  return it[btnFocus_].type == t && it[btnFocus_].slot == slot;
}

int8_t DisplayUI::btnFocusSlot(TouchEvent::Type t) const {
  uint8_t n = 0;
  const FocusItem* it = focusTable(screen_, &n);
  if (it != nullptr && btnFocus_ < n && it[btnFocus_].type == t) {
    return static_cast<int8_t>(it[btnFocus_].slot);
  }
  if (touchDown_ && heldType_ == t) {
    return static_cast<int8_t>(sstvPressSlot_);
  }
  return -1;
}

void DisplayUI::nudgeBtnFocus(int8_t dir) {
  const uint8_t n = btnFocusCount();
  if (n == 0) {
    return;
  }
  int next = static_cast<int>(btnFocus_) + dir;
  if (next < 0) {
    next = n - 1;
  }
  if (next >= static_cast<int>(n)) {
    next = 0;
  }
  btnFocus_ = static_cast<uint8_t>(next);
  refreshChrome();
}

TouchEvent DisplayUI::focusEvent() const {
  TouchEvent ev;
  ev.type = TouchEvent::None;
  ev.bandIndex = 0;
  uint8_t n = 0;
  const FocusItem* it = focusTable(screen_, &n);
  if (it == nullptr || btnFocus_ >= n) {
    return ev;
  }
  ev.type = it[btnFocus_].type;
  ev.bandIndex = it[btnFocus_].slot;
  return ev;
}

void DisplayUI::refreshChrome() { redrawTouchChrome(); }

void DisplayUI::calibrate() {
  if (!ready_) {
    return;
  }
  runTouchCalibration();
  touchDown_ = false;
  heldType_ = TouchEvent::None;
  dirty_ = kFull;
  last_ = UiState();
}

void DisplayUI::begin() {
  tft.init();
  tft.setRotation(FENNEC_TFT_ROTATION);
#if FENNEC_TFT_INVERT
  tft.invertDisplay(true);
#endif
  backlightAttach();
  brightness_ = FENNEC_DEFAULT_BRIGHTNESS;
  setBrightness(brightness_);
  pinMode(PIN_TOUCH_CS, OUTPUT);
  digitalWrite(PIN_TOUCH_CS, HIGH);

  touchBusBegin();
  ts.begin(touchSPI);
  touchBusBegin();
  ts.setRotation(1);
  loadTouchCal();

  tft.fillScreen(theme::bg);
  huntSpr.setColorDepth(16);
  huntSpr.createSprite(kHuntSprW, kHuntSprH);
  ready_ = true;
  dirty_ = kFull;
}

void DisplayUI::showSplash() {
  if (!ready_) {
    return;
  }
  tft.fillScreen(theme::bg);
  drawBrackets(6, 314, 7);

  const int16_t logoX = static_cast<int16_t>((tft.width() - kCiferLogoW) / 2);
  const int16_t logoY = 20;
  tft.drawBitmap(logoX, logoY, kCiferLogo, kCiferLogoW, kCiferLogoH, theme::sand);
  constexpr int16_t kTickPad = 4;
  drawCornerTicks(static_cast<int16_t>(logoX - kTickPad), static_cast<int16_t>(logoY - kTickPad),
                  static_cast<int16_t>(logoX + kCiferLogoW + kTickPad - 1),
                  static_cast<int16_t>(logoY + kCiferLogoH + kTickPad - 1), 12);

  drawSpaced("FENNEC", 120, 198, 4, 4, theme::sand);
  drawSpaced(FENNEC_VERSION, 120, 222, 2, 1, theme::dim);
  tft.drawFastHLine(40, 238, 160, theme::dim);
  drawSpaced("FM  AM  SW", 120, 256, 3, 1, theme::muted);
  drawSpaced("BY CiferTech", 120, 286, 2, 2, theme::line);
  delay(1800);
}

bool DisplayUI::noSignal() const { return !cur_.present || cur_.rssi < 8; }

void DisplayUI::apply(const UiState& state) {
  cur_ = state;
  if (!ready_ || screen_ != UiScreen::Tuner) {
    return;
  }
  if (last_.bandIndex != cur_.bandIndex) {
    dirty_ |= kFull;
    return;
  }
  if (last_.frequency != cur_.frequency) {
    dirty_ |= kFreq;
  }
  if (last_.rssi != cur_.rssi || last_.snr != cur_.snr) {
    dirty_ |= kMeter;
    if (noSignal() != (!last_.present || last_.rssi < 8)) {
      dirty_ |= kFreq | kDial;
    }
  }
  if (last_.volume != cur_.volume) {
    dirty_ |= kVol;
  }
  if (last_.muted != cur_.muted) {
    dirty_ |= kVol | kKeys | kHead;
  }
  if (last_.stereo != cur_.stereo) {
    dirty_ |= kFreq;
  }
  if (last_.wifi != cur_.wifi || strncmp(last_.clock, cur_.clock, sizeof(cur_.clock)) != 0) {
    dirty_ |= kHead;
  }
  if (last_.present != cur_.present) {
    dirty_ |= kHead | kDial | kFreq;
  }
  if (strncmp(last_.rds, cur_.rds, sizeof(cur_.rds)) != 0) {
    dirty_ |= kDial;
  }
}

void DisplayUI::poll() {
  if (!ready_ || screen_ != UiScreen::Tuner) {
    return;
  }
  if (dirty_ == 0) {
    return;
  }
  if (dirty_ & kFull) {
    drawFull();
    last_ = cur_;
    dirty_ = 0;
    return;
  }
  if (dirty_ & kHead) {
    drawHeader();
  }
  if (dirty_ & kSection) {
    drawSection();
  }
  if (dirty_ & kFreq) {
    drawFrequency();
  }
  if (dirty_ & kDial) {
    drawDial();
  }
  if (dirty_ & (kMeter | kVol)) {
    drawMeters();
  }
  if (dirty_ & kKeys) {
    drawKeys();
  }
  last_ = cur_;
  dirty_ = 0;
}

bool DisplayUI::pollTouch(TouchEvent& ev) {
  ev.type = TouchEvent::None;
  ev.bandIndex = 0;
  if (!ready_) {
    return false;
  }

  int16_t tx = 0;
  int16_t ty = 0;
  const bool pressed = readTouch(&tx, &ty);
  const uint32_t now = millis();

  if (eatTouch_ || now < touchLockUntil_) {
    if (!pressed) {
      eatTouch_ = false;
      touchDown_ = false;
      heldType_ = TouchEvent::None;
      if (screen_ == UiScreen::Settings) {
        paintSettingsRows();
      }
    }
    return false;
  }

  if (!pressed) {
    if (touchDown_) {
      touchDown_ = false;
      heldType_ = TouchEvent::None;
      if (screen_ != UiScreen::Settings) {
        redrawTouchChrome();
      } else {
        paintSettingsRows();
      }
    } else {
      touchDown_ = false;
      heldType_ = TouchEvent::None;
    }
    return false;
  }

  uint8_t band = 0;
  const TouchEvent::Type hit = hitAt(tx, ty, &band);
  if (hit == TouchEvent::None) {
    return false;
  }

  const bool repeatable =
      hit == TouchEvent::TuneUp || hit == TouchEvent::TuneDown || hit == TouchEvent::VolUp ||
      hit == TouchEvent::VolDown || hit == TouchEvent::SstvBfoDown || hit == TouchEvent::SstvBfoUp ||
      hit == TouchEvent::CwPitchDown || hit == TouchEvent::CwPitchUp ||
      hit == TouchEvent::LogThrDown || hit == TouchEvent::LogThrUp;

  if (!touchDown_) {
    touchDown_ = true;
    touchRepeatMs_ = now;
    heldType_ = hit;
    sstvPressSlot_ = band;
    if (screen_ != UiScreen::Settings) {
      redrawTouchChrome();
    } else {
      paintSettingsRows();
    }
    ev.type = hit;
    ev.bandIndex = band;
    return true;
  }

  if (repeatable && hit == heldType_ && (now - touchRepeatMs_ >= FENNEC_TOUCH_REPEAT_MS)) {
    touchRepeatMs_ = now;
    ev.type = hit;
    ev.bandIndex = band;
    return true;
  }
  return false;
}

void DisplayUI::lockTouch() {
  eatTouch_ = true;
  touchLockUntil_ = millis() + 400;
}

void DisplayUI::drawFull() {
  tft.fillScreen(theme::bg);
  drawBrackets(2, 16);
  drawHeader();
  drawTabs();
  drawSection();
  drawFrequency();
  drawDial();
  drawMeters();
  drawKeys();
}

void DisplayUI::drawHeader() {
  tft.fillRect(14, 2, 212, 15, theme::bg);
  const char* left = "NC";
  if (cur_.muted) {
    left = "MUTE";
  } else if (cur_.present) {
    left = "SIG";
  }
  drawSignalGlyph(14, 13, cur_.rssi, cur_.present && !cur_.muted);
  drawSpacedLeft(left, 34, 9, 2, 1, cur_.present && !cur_.muted ? theme::sand : theme::muted);

  if (cur_.clock[0]) {
    drawSpaced(cur_.clock, 120, 9, 1, 1, theme::dim);
  }

  const BandKind kind = bandAt(cur_.bandIndex).kind;
  const char* right = kind == BandKind::FM ? "FM" : (kind == BandKind::AM ? "AM" : "SW");
  drawSpacedRight(right, 228, 9, 3, 1, theme::sand);
}

void DisplayUI::drawTabs() {
  const BandKind kind = bandAt(cur_.bandIndex).kind;
  const TouchEvent::Type press = touchDown_ ? heldType_ : TouchEvent::None;
  drawTab(kTabFm, "FM", kind == BandKind::FM || press == TouchEvent::TabFm);
  drawTab(kTabMw, "AM", kind == BandKind::AM || press == TouchEvent::TabMw);
  drawTab(kTabSw, "SW", kind == BandKind::SW || press == TouchEvent::TabSw);
  drawTab(kTabMore, "MENU", press == TouchEvent::TabMore);
}

void DisplayUI::drawSection() {
  const Band& b = bandAt(cur_.bandIndex);
  tft.fillRect(4, 56, 232, 22, theme::bg);

  const TouchEvent::Type press = touchDown_ ? heldType_ : TouchEvent::None;
  for (uint8_t i = 0; i < 3; ++i) {
    const uint8_t idx = compactBandIndex(b.kind, cur_.bandIndex, i);
    drawOutlineKey(kBandCompact[i], kBands[idx].name, i == 1 || (press == TouchEvent::SwBand && sstvPressSlot_ == idx));
  }
}

void DisplayUI::drawFrequency() {
  char buf[16];
  formatFreq(buf, sizeof(buf));
  const Band& b = bandAt(cur_.bandIndex);

  static size_t lastLen = 0;
  const size_t len = strlen(buf);
  if (len != lastLen) {
    tft.fillRect(6, 94, 228, 52, theme::bg);
    lastLen = len;
  }
  useBitmapFont(kFontBig);
  tft.setTextDatum(MC_DATUM);
  const uint16_t fg = noSignal() ? theme::dim : theme::sand;
  tft.setTextColor(fg, theme::bg);
  tft.drawString(buf, 120, 120, kFontBig);

  tft.fillRect(6, 148, 228, 12, theme::bg);
  if (noSignal()) {
    drawSpaced("NO SIG", 120, 154, 3, 1, theme::dim);
    return;
  }
  char unit[28];
  if (b.kind == BandKind::FM) {
    snprintf(unit, sizeof(unit), "MHZ  FM%s", cur_.stereo ? "  ST" : "");
  } else {
    snprintf(unit, sizeof(unit), "KHZ  %s", b.kind == BandKind::AM ? "AM" : "SW");
  }
  drawSpaced(unit, 120, 154, 3, 1, theme::muted);
}

void DisplayUI::drawDial() {
  const Band& b = bandAt(cur_.bandIndex);
  tft.fillRect(8, 168, 224, 72, theme::bg);
  tft.drawFastHLine(20, 172, 200, theme::dim);
  tft.drawFastHLine(20, 236, 200, theme::dim);

  char range[28];
  if (b.kind == BandKind::FM) {
    snprintf(range, sizeof(range), "%u.%02u-%u.%02u MHZ", b.minFreq / 100, b.minFreq % 100, b.maxFreq / 100,
             b.maxFreq % 100);
  } else if (b.maxFreq >= 10000) {
    snprintf(range, sizeof(range), "%u.%u-%u.%u MHZ", b.minFreq / 1000, (b.minFreq % 1000) / 100,
             b.maxFreq / 1000, (b.maxFreq % 1000) / 100);
  } else {
    snprintf(range, sizeof(range), "%u-%u KHZ", b.minFreq, b.maxFreq);
  }

  char state[20];
  if (cur_.rds[0]) {
    snprintf(state, sizeof(state), "%.8s", cur_.rds);
  } else {
    snprintf(state, sizeof(state), b.kind == BandKind::FM ? "STEP %u0 KHZ" : "STEP %u KHZ", b.step);
  }

  drawSpacedLeft("BAND", 22, 184, 2, 1, theme::muted);
  drawSpacedLeft("RANGE", 22, 204, 2, 1, theme::muted);
  drawSpacedLeft("STATE", 22, 224, 2, 1, theme::muted);

  useBitmapFont(2);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(noSignal() ? theme::muted : theme::sand, theme::bg);
  tft.drawString(b.name, 94, 184, 2);
  useBitmapFont(1);
  tft.setTextColor(theme::muted, theme::bg);
  tft.drawString(range, 94, 204, 1);
  tft.setTextColor(theme::dim, theme::bg);
  tft.drawString(state, 94, 224, 1);
}

void DisplayUI::drawMeters() {
  constexpr int16_t kY = 242;
  constexpr int16_t kH = 20;
  constexpr int16_t kCy = 252;
  constexpr int16_t kBarH = 8;
  constexpr int16_t kBarY = kCy - kBarH / 2;
  constexpr int16_t kBarW = 4;
  constexpr int16_t kBarGap = 2;
  constexpr int16_t kPitch = kBarW + kBarGap;
  constexpr uint8_t kSigBars = 10;
  constexpr uint8_t kVolBars = 8;
  constexpr int16_t kPad = 6;
  constexpr int16_t kMid = 14;

  tft.fillRect(8, kY, 224, kH, theme::bg);

  const int16_t sigW = spacedWidth(tft, "SIG", 2, 1);
  const int16_t volW = spacedWidth(tft, "VOL", 2, 1);
  char dbu[8];
  snprintf(dbu, sizeof(dbu), "%02u", cur_.rssi);
  const int16_t numW = spacedWidth(tft, dbu, 2, 1);
  const int16_t sigBarsW = static_cast<int16_t>(kSigBars * kPitch - kBarGap);
  const int16_t volBarsW = static_cast<int16_t>(kVolBars * kPitch - kBarGap);
  const int16_t total =
      sigW + kPad + sigBarsW + kPad + numW + kMid + volW + kPad + volBarsW;
  int16_t x = static_cast<int16_t>(8 + (224 - total) / 2);

  drawSpacedLeft("SIG", x, kCy, 2, 1, theme::muted);
  x += sigW + kPad;
  const uint8_t rssi = cur_.rssi > 70 ? 70 : cur_.rssi;
  const uint8_t sigLit = static_cast<uint8_t>(rssi * kSigBars / 70);
  for (uint8_t i = 0; i < kSigBars; ++i) {
    tft.fillRect(x + static_cast<int16_t>(i * kPitch), kBarY, kBarW, kBarH,
                 i < sigLit ? theme::sand : theme::dim);
  }
  x += sigBarsW + kPad;
  drawSpacedLeft(dbu, x, kCy, 2, 1, cur_.present ? theme::line : theme::dim);
  x += numW + kMid;
  drawSpacedLeft("VOL", x, kCy, 2, 1, theme::muted);
  x += volW + kPad;
  const uint8_t volLit = cur_.muted ? 0 : static_cast<uint8_t>(cur_.volume * kVolBars / 63);
  for (uint8_t i = 0; i < kVolBars; ++i) {
    tft.fillRect(x + static_cast<int16_t>(i * kPitch), kBarY, kBarW, kBarH,
                 i < volLit ? theme::sand : theme::dim);
  }

  drawSignalGlyph(14, 13, cur_.rssi, cur_.present && !cur_.muted);
}

void DisplayUI::drawKeys() {
  const TouchEvent::Type press = touchDown_ ? heldType_ : TouchEvent::None;
  drawOutlineKey(kBtnTuneDn, "-", press == TouchEvent::TuneDown);
  drawOutlineKey(kBtnSeek, "SEEK", press == TouchEvent::Seek);
  drawOutlineKey(kBtnTuneUp, "+", press == TouchEvent::TuneUp);
  drawOutlineKey(kBtnVolDn, "VOL-", press == TouchEvent::VolDown);
  drawOutlineKey(kBtnMute, cur_.muted ? "UNMUTE" : "MUTE", cur_.muted || press == TouchEvent::Mute);
  drawOutlineKey(kBtnVolUp, "VOL+", press == TouchEvent::VolUp);
}

void DisplayUI::redrawTouchChrome() {
  switch (screen_) {
    case UiScreen::Menu:
      drawMenu();
      break;
    case UiScreen::Tuner:
      drawTabs();
      drawSection();
      drawKeys();
      break;
    case UiScreen::Sstv:
      drawSstvKeys();
      break;
    case UiScreen::Hunt: {
      drawOutlineKey(kSstvBack, "BACK", btnLit(TouchEvent::SstvBack));
      drawOutlineKey(kHuntPeak, "PEAK", btnLit(TouchEvent::HuntPeak));
      break;
    }
    case UiScreen::Cw: {
      drawOutlineKey(kSstvBack, "BACK", btnLit(TouchEvent::SstvBack));
      drawOutlineKey(kSstvUsb, cwUsbShown ? "USB" : "LSB", true);
      drawOutlineKey(kCwSrc, cwMicShown ? "MIC" : "RADIO", cwMicShown || btnLit(TouchEvent::CwSrc));
      drawOutlineKey(kCwPitchDn, "PIT-", btnLit(TouchEvent::CwPitchDown));
      drawOutlineKey(kCwPitchUp, "PIT+", btnLit(TouchEvent::CwPitchUp));
      drawOutlineKey(kCwBfoDn, "BFO-", btnLit(TouchEvent::SstvBfoDown));
      drawOutlineKey(kCwBfoUp, "BFO+", btnLit(TouchEvent::SstvBfoUp));
      drawOutlineKey(kCwVolDn, "VOL-", btnLit(TouchEvent::VolDown));
      drawOutlineKey(kCwVolUp, "VOL+", btnLit(TouchEvent::VolUp));
      break;
    }
    case UiScreen::Log: {
      drawOutlineKey(kSstvBack, "BACK", btnLit(TouchEvent::SstvBack));
      drawOutlineKey(kSstvUsb, logSsb_ ? (logUsb_ ? "USB" : "LSB") : "AM", true);
      drawOutlineKey(kLogFreqDn, "-", btnLit(TouchEvent::TuneDown));
      drawOutlineKey(kLogFreqUp, "+", btnLit(TouchEvent::TuneUp));
      drawOutlineKey(kLogThrDn, "THR-", btnLit(TouchEvent::LogThrDown));
      drawOutlineKey(kLogArm, logArmed_ ? "ARM" : "IDLE", logArmed_ || btnLit(TouchEvent::LogArm));
      drawOutlineKey(kLogClr, "CLR", btnLit(TouchEvent::LogClr));
      drawOutlineKey(kLogThrUp, "THR+", btnLit(TouchEvent::LogThrUp));
      drawLogPresetRow(logPreset_, btnFocusSlot(TouchEvent::LogPreset));
      break;
    }
    case UiScreen::About:
      drawAbout();
      break;
    case UiScreen::Settings:
      drawSettings();
      break;
  }
}

void DisplayUI::formatFreq(char* out, size_t n) const {
  const Band& b = bandAt(cur_.bandIndex);
  if (b.kind == BandKind::FM) {
    snprintf(out, n, "%u.%02u", cur_.frequency / 100, cur_.frequency % 100);
  } else {
    snprintf(out, n, "%u", cur_.frequency);
  }
}

TouchEvent::Type DisplayUI::hitAt(int16_t x, int16_t y, uint8_t* bandOut) const {
  *bandOut = 0;
  if (screen_ == UiScreen::Sstv) {
    if (inside(x, y, kSstvBack)) {
      return TouchEvent::SstvBack;
    }
    if (inside(x, y, kSstvUsb)) {
      return TouchEvent::SstvUsb;
    }
    if (inside(x, y, kSstvSrc)) {
      return TouchEvent::SstvSrc;
    }
    if (inside(x, y, kSstvMode)) {
      return TouchEvent::SstvCycleMode;
    }
    for (uint8_t i = 0; i < 4; ++i) {
      if (inside(x, y, kSstvCall[i])) {
        *bandOut = i;
        return TouchEvent::SstvCall;
      }
    }
    if (inside(x, y, kSstvVolDn)) {
      return TouchEvent::VolDown;
    }
    if (inside(x, y, kSstvVolUp)) {
      return TouchEvent::VolUp;
    }
    if (inside(x, y, kSstvBfoDn)) {
      return TouchEvent::SstvBfoDown;
    }
    if (inside(x, y, kSstvBfoUp)) {
      return TouchEvent::SstvBfoUp;
    }
    if (inside(x, y, kSstvReset)) {
      return TouchEvent::SstvReset;
    }
    return TouchEvent::None;
  }
  if (screen_ == UiScreen::Menu) {
    if (inside(x, y, kMenuAbout)) {
      return TouchEvent::OpenAbout;
    }
    if (inside(x, y, kMenuSettings)) {
      return TouchEvent::OpenSettings;
    }
    for (uint8_t i = 0; i < 5; ++i) {
      if (inside(x, y, kMenuTile[i])) {
        *bandOut = i;
        return TouchEvent::MorePick;
      }
    }
    return TouchEvent::None;
  }
  if (screen_ == UiScreen::About) {
    return TouchEvent::SstvBack;
  }
  if (screen_ == UiScreen::Settings) {
    if (inside(x, y, kHintBar)) {
      return TouchEvent::SstvBack;
    }
    for (uint8_t i = 0; i < 4; ++i) {
      if (inside(x, y, kMenuTile[i])) {
        *bandOut = i;
        return TouchEvent::SettingsPick;
      }
    }
    return TouchEvent::None;
  }
  if (screen_ == UiScreen::Hunt) {
    if (inside(x, y, kSstvBack)) {
      return TouchEvent::SstvBack;
    }
    if (inside(x, y, kHuntPeak)) {
      return TouchEvent::HuntPeak;
    }
    return TouchEvent::None;
  }
  if (screen_ == UiScreen::Cw) {
    if (inside(x, y, kSstvBack)) {
      return TouchEvent::SstvBack;
    }
    if (inside(x, y, kSstvUsb)) {
      return TouchEvent::SstvUsb;
    }
    if (inside(x, y, kCwSrc)) {
      return TouchEvent::CwSrc;
    }
    if (inside(x, y, kCwPitchDn)) {
      return TouchEvent::CwPitchDown;
    }
    if (inside(x, y, kCwPitchUp)) {
      return TouchEvent::CwPitchUp;
    }
    if (inside(x, y, kCwBfoDn)) {
      return TouchEvent::SstvBfoDown;
    }
    if (inside(x, y, kCwBfoUp)) {
      return TouchEvent::SstvBfoUp;
    }
    if (inside(x, y, kCwVolDn)) {
      return TouchEvent::VolDown;
    }
    if (inside(x, y, kCwVolUp)) {
      return TouchEvent::VolUp;
    }
    return TouchEvent::None;
  }
  if (screen_ == UiScreen::Log) {
    if (inside(x, y, kSstvBack)) {
      return TouchEvent::SstvBack;
    }
    if (inside(x, y, kLogArm)) {
      return TouchEvent::LogArm;
    }
    if (inside(x, y, kLogFreqDn)) {
      return TouchEvent::TuneDown;
    }
    if (inside(x, y, kLogFreqUp)) {
      return TouchEvent::TuneUp;
    }
    if (inside(x, y, kSstvUsb)) {
      return TouchEvent::SstvUsb;
    }
    for (uint8_t i = 0; i < 5; ++i) {
      if (inside(x, y, kLogPreset[i])) {
        *bandOut = i;
        return TouchEvent::LogPreset;
      }
    }
    if (inside(x, y, kLogThrDn)) {
      return TouchEvent::LogThrDown;
    }
    if (inside(x, y, kLogThrUp)) {
      return TouchEvent::LogThrUp;
    }
    if (inside(x, y, kLogClr)) {
      return TouchEvent::LogClr;
    }
    return TouchEvent::None;
  }
  if (inside(x, y, kTabFm)) {
    return TouchEvent::TabFm;
  }
  if (inside(x, y, kTabMw)) {
    return TouchEvent::TabMw;
  }
  if (inside(x, y, kTabSw)) {
    return TouchEvent::TabSw;
  }
  if (inside(x, y, kTabMore)) {
    return TouchEvent::TabMore;
  }
  {
    const BandKind kind = bandAt(cur_.bandIndex).kind;
    for (uint8_t i = 0; i < 3; ++i) {
      if (inside(x, y, kBandCompact[i])) {
        *bandOut = compactBandIndex(kind, cur_.bandIndex, i);
        return TouchEvent::SwBand;
      }
    }
  }
  if (inside(x, y, kBtnTuneDn)) {
    return TouchEvent::TuneDown;
  }
  if (inside(x, y, kBtnTuneUp)) {
    return TouchEvent::TuneUp;
  }
  if (inside(x, y, kBtnSeek)) {
    return TouchEvent::Seek;
  }
  if (inside(x, y, kBtnMute)) {
    return TouchEvent::Mute;
  }
  if (inside(x, y, kBtnVolDn)) {
    return TouchEvent::VolDown;
  }
  if (inside(x, y, kBtnVolUp)) {
    return TouchEvent::VolUp;
  }
  return TouchEvent::None;
}

void DisplayUI::enterSstv() {
  screen_ = UiScreen::Sstv;
  sstvUsb_ = true;
  sstvSlot_ = 2;
  sstvBfo_ = 0;
  sstvPressSlot_ = 0;
  btnFocus_ = 0;
  heldType_ = TouchEvent::None;
  dirty_ = 0;
  ++sstvHudEpoch;
  tft.fillScreen(theme::bg);
  tft.fillRect(kSstvPic.x, kSstvPic.y, kSstvPic.w, kSstvPic.h, 0x0000);
  tft.drawRect(kSstvPic.x - 1, kSstvPic.y - 1, kSstvPic.w + 2, kSstvPic.h + 2, theme::line);
  drawSstvChrome();
}

void DisplayUI::exitOverlay() {
  screen_ = UiScreen::Tuner;
  toolChrome_ = false;
  dirty_ = kFull;
}

void DisplayUI::drawSstvChrome() {
  drawToolTitle("SSTV");
  drawSstvKeys();
}

void DisplayUI::drawSstvKeys() {
  if (!sstvMode()) {
    return;
  }
  drawOutlineKey(kSstvBack, "BACK", btnLit(TouchEvent::SstvBack));
  drawOutlineKey(kSstvUsb, sstvUsb_ ? "USB" : "LSB", true);
  drawOutlineKey(kSstvSrc, sstvMic_ ? "MIC" : "RADIO", sstvMic_ || btnLit(TouchEvent::SstvSrc));
  drawOutlineKey(kSstvMode, sstvLockLabel(sstvShown_, sstvPrefer_, false),
                 sstvPrefer_ != SstvMode::Unknown || btnLit(TouchEvent::SstvCycleMode));
  drawSstvCallRow(sstvSlot_, btnFocusSlot(TouchEvent::SstvCall));
  drawOutlineKey(kSstvVolDn, "VOL-", btnLit(TouchEvent::VolDown));
  drawOutlineKey(kSstvBfoDn, "BFO-", btnLit(TouchEvent::SstvBfoDown));
  drawOutlineKey(kSstvReset, "RESET", btnLit(TouchEvent::SstvReset));
  drawOutlineKey(kSstvBfoUp, "BFO+", btnLit(TouchEvent::SstvBfoUp));
  drawOutlineKey(kSstvVolUp, "VOL+", btnLit(TouchEvent::VolUp));
}

void DisplayUI::sstvBlit(const SstvLine& line) {
  if (!sstvMode() || line.y >= kSstvOutH) {
    return;
  }
  tft.pushImage(kSstvPic.x, static_cast<int32_t>(kSstvPic.y + line.y), kSstvOutW, 1, line.px);
  tft.drawFastVLine(kSstvPic.x, kSstvPic.y + static_cast<int16_t>(line.y), 1, theme::sand);
}

void DisplayUI::setSstvMic(bool mic) {
  sstvMic_ = mic;
  if (sstvMode()) {
    drawOutlineKey(kSstvSrc, sstvMic_ ? "MIC" : "RADIO", sstvMic_ || btnLit(TouchEvent::SstvSrc));
  }
}

void DisplayUI::sstvHud(const SstvStatus& st, uint16_t freqKhz, bool usb, int16_t bfo, uint8_t volume,
                       bool mic) {
  if (!sstvMode()) {
    return;
  }
  const uint8_t nextSlot = sstvSlotFromFreq(freqKhz);
  if (sstvUsb_ != usb) {
    sstvUsb_ = usb;
    drawOutlineKey(kSstvUsb, usb ? "USB" : "LSB", true);
  }
  if (sstvMic_ != mic) {
    setSstvMic(mic);
  }
  if (sstvSlot_ != nextSlot) {
    sstvSlot_ = nextSlot;
    drawSstvCallRow(sstvSlot_, btnFocusSlot(TouchEvent::SstvCall));
  }
  sstvBfo_ = bfo;

  static uint8_t epoch = 255;
  static uint8_t lastMode = 255;
  static uint16_t lastLine = 0xFFFF;
  static uint16_t lastLines = 0xFFFF;
  static uint16_t lastFreq = 0;
  static int16_t lastBfo = INT16_MIN;
  static uint8_t lastVol = 255;
  static bool lastUsb = false;
  static bool lastMic = true;
  static bool lastAudio = true;
  static bool lastFin = true;
  static uint8_t lastPeak = 255;
  static uint8_t lastPrefer = 255;
  const uint8_t peakNibble = static_cast<uint8_t>(st.peak >> 7);
  if (epoch == sstvHudEpoch && lastMode == static_cast<uint8_t>(st.mode) && lastLine == st.line &&
      lastLines == st.lines && lastFreq == freqKhz && lastBfo == bfo && lastVol == volume &&
      lastUsb == usb && lastMic == mic && lastAudio == st.hasAudio && lastFin == st.finished &&
      lastPeak == peakNibble && lastPrefer == static_cast<uint8_t>(st.prefer)) {
    return;
  }
  epoch = sstvHudEpoch;
  lastMode = static_cast<uint8_t>(st.mode);
  lastLine = st.line;
  lastLines = st.lines;
  lastFreq = freqKhz;
  lastBfo = bfo;
  lastVol = volume;
  lastUsb = usb;
  lastMic = mic;
  lastAudio = st.hasAudio;
  lastFin = st.finished;
  lastPeak = peakNibble;
  lastPrefer = static_cast<uint8_t>(st.prefer);

  sstvShown_ = st.mode;
  sstvPrefer_ = st.prefer;
  drawOutlineKey(kSstvMode, sstvLockLabel(st.mode, st.prefer, st.finished),
                 st.prefer != SstvMode::Unknown || btnLit(TouchEvent::SstvCycleMode));

  constexpr int16_t kBarX = 156;
  constexpr int16_t kBarW = 76;
  constexpr int16_t kBarH = 6;
  const int16_t kBarY = static_cast<int16_t>(kSstvMode.y + kSstvMode.h - kBarH);
  tft.fillRect(kBarX, kSstvMode.y, kBarW, kSstvMode.h, theme::bg);

  char tune[28];
  snprintf(tune, sizeof(tune), "%u  %+d", freqKhz, static_cast<int>(bfo));
  drawSpacedRight(tune, 230, 223, 1, 1, theme::muted);

  tft.fillRect(kBarX, kBarY, kBarW, kBarH, theme::dim);
  uint16_t fill = 0;
  if (st.finished && st.lines > 0) {
    fill = kBarW;
  } else if (st.mode != SstvMode::Unknown && st.lines > 0) {
    fill = static_cast<uint16_t>(st.line) * kBarW / st.lines;
  } else {
    fill = st.peak > 800 ? kBarW : static_cast<uint16_t>(st.peak) * kBarW / 800;
  }
  if (fill > kBarW) {
    fill = kBarW;
  }
  if (fill > 0) {
    tft.fillRect(kBarX, kBarY, static_cast<int16_t>(fill), kBarH, st.hasAudio ? theme::sand : theme::line);
  }
}

void DisplayUI::enterMenu() {
  screen_ = UiScreen::Menu;
  btnFocus_ = 0;
  tft.fillScreen(theme::bg);
  drawMenu();
}

void DisplayUI::enterHunt() {
  screen_ = UiScreen::Hunt;
  btnFocus_ = 0;
  toolChrome_ = false;
  tft.fillScreen(theme::bg);
}

void DisplayUI::enterCw() {
  screen_ = UiScreen::Cw;
  btnFocus_ = 0;
  toolChrome_ = false;
  tft.fillScreen(theme::bg);
}

void DisplayUI::enterLog() {
  screen_ = UiScreen::Log;
  btnFocus_ = 0;
  toolChrome_ = false;
  tft.fillScreen(theme::bg);
}

void DisplayUI::enterAbout() {
  screen_ = UiScreen::About;
  btnFocus_ = 0;
  toolChrome_ = false;
  tft.fillScreen(theme::bg);
  drawAbout();
}

void DisplayUI::enterSettings(bool wifiOn, bool pixelOn, uint8_t brightness, const char* ip,
                             bool wipe) {
  const bool fresh = screen_ != UiScreen::Settings;
  screen_ = UiScreen::Settings;
  if (fresh || wipe) {
    btnFocus_ = 0;
  }
  toolChrome_ = false;
  settingsWifi_ = wifiOn;
  settingsPixel_ = pixelOn;
  settingsBright_ = brightness;
  settingsIp_[0] = 0;
  if (ip != nullptr) {
    strncpy(settingsIp_, ip, sizeof(settingsIp_) - 1);
    settingsIp_[sizeof(settingsIp_) - 1] = 0;
  }
  if (fresh || wipe) {
    tft.fillScreen(theme::bg);
    drawSettings();
  } else {
    paintSettingsRows();
    paintSettingsIp();
  }
}

void DisplayUI::drawMenu() {
  drawToolTitle("FENNEC");
  drawIconKey(kMenuAbout, kIconAbout, btnLit(TouchEvent::OpenAbout));
  drawIconKey(kMenuSettings, kIconSetting, btnLit(TouchEvent::OpenSettings), true);
  const char* names[5] = {"RADIO", "SSTV", "MORSE", "HOMING", "WATCH"};
  const char* sub[5] = {"LISTEN", "PICTURE", "COPY", "PEAK", "EVENTS"};
  constexpr int16_t kGutter = 40;
  constexpr int16_t kNamePad = 14;
  constexpr int16_t kSubPad = 14;
  constexpr int16_t kRuleInset = 8;

  for (uint8_t i = 0; i < 5; ++i) {
    const Rect& r = kMenuTile[i];
    const bool on = btnLit(TouchEvent::MorePick, i);
    const uint16_t stroke = on ? theme::sand : theme::dim;
    const uint16_t nameCol = on ? theme::sand : theme::line;
    const uint16_t metaCol = on ? theme::sand : theme::muted;
    const int16_t cy = static_cast<int16_t>(r.y + r.h / 2);

    tft.fillRect(r.x, r.y, r.w, r.h, theme::bg);
    tft.drawRect(r.x, r.y, r.w, r.h, stroke);
    tft.drawFastVLine(r.x + kGutter, r.y + kRuleInset, r.h - 2 * kRuleInset, stroke);

    char idx[4];
    snprintf(idx, sizeof(idx), "%02u", static_cast<unsigned>(i + 1));
    drawSpaced(idx, static_cast<int16_t>(r.x + kGutter / 2), cy, 1, 1, metaCol);
    drawSpacedLeft(names[i], static_cast<int16_t>(r.x + kGutter + kNamePad), cy, 2, 2, nameCol);
    drawSpacedRight(sub[i], static_cast<int16_t>(r.x + r.w - kSubPad), cy, 1, 1, metaCol);
  }

  drawBrackets(304, 316);
  drawSpaced("TAP OR OK", 120, 310, 1, 1, theme::muted);
}

void DisplayUI::drawAbout() {
  drawToolTitle("ABOUT");

  drawSpaced("FENNEC", 120, 56, 4, 4, theme::sand);
  drawSpaced("by CiferTech", 120, 80, 1, 1, theme::line);
  drawSpaced(FENNEC_VERSION, 120, 98, 2, 1, theme::dim);
  tft.drawFastHLine(16, 112, 208, theme::dim);

  drawInfoRow(132, "Board", "ESP32-S3");
  drawInfoRow(154, "Radio", "SI4732");
  drawInfoRow(176, "Mail", "cifertech@gmail.com");
  drawInfoRow(198, "GitHub", "github.com/cifertech");
  drawInfoRow(220, "Web", "cifertech.net");

  drawBrackets(304, 316);
  drawSpaced("OK OR TAP BACK", 120, 310, 1, 1, theme::muted);
}

void DisplayUI::drawSettings() {
  drawToolTitle("SETTINGS");
  paintSettingsRows();
  paintSettingsIp();
  drawBrackets(304, 316);
  drawSpaced("BACK", 120, 310, 1, 1, btnLit(TouchEvent::SstvBack) ? theme::sand : theme::muted);
}

void DisplayUI::paintSettingsRows() {
  char br[8];
  snprintf(br, sizeof(br), "%u%%", static_cast<unsigned>(settingsBright_));
  const char* names[4] = {"WIFI AP", "PIXEL", "BRIGHT", "TOUCH"};
  const char* vals[4] = {settingsWifi_ ? "ON" : "OFF", settingsPixel_ ? "ON" : "OFF", br, "CAL"};
  constexpr int16_t kGutter = 40;
  constexpr int16_t kNamePad = 14;
  constexpr int16_t kSubPad = 14;
  constexpr int16_t kRuleInset = 8;

  for (uint8_t i = 0; i < 4; ++i) {
    const Rect& r = kMenuTile[i];
    const bool on = btnLit(TouchEvent::SettingsPick, i);
    const uint16_t stroke = on ? theme::sand : theme::dim;
    const uint16_t nameCol = on ? theme::sand : theme::line;
    const uint16_t metaCol = on ? theme::sand : theme::muted;
    const int16_t cy = static_cast<int16_t>(r.y + r.h / 2);

    tft.fillRect(r.x, r.y, r.w, r.h, theme::bg);
    tft.drawRect(r.x, r.y, r.w, r.h, stroke);
    tft.drawFastVLine(r.x + kGutter, r.y + kRuleInset, r.h - 2 * kRuleInset, stroke);

    char idx[4];
    snprintf(idx, sizeof(idx), "%02u", static_cast<unsigned>(i + 1));
    drawSpaced(idx, static_cast<int16_t>(r.x + kGutter / 2), cy, 1, 1, metaCol);
    drawSpacedLeft(names[i], static_cast<int16_t>(r.x + kGutter + kNamePad), cy, 2, 2, nameCol);
    drawSpacedRight(vals[i], static_cast<int16_t>(r.x + r.w - kSubPad), cy, 1, 1, metaCol);
  }
}

void DisplayUI::paintSettingsIp() {
  tft.fillRect(12, 246, 216, 18, theme::bg);
  if (settingsWifi_ && settingsIp_[0]) {
    drawSpaced(settingsIp_, 120, 256, 1, 1, theme::dim);
  }
}

void DisplayUI::huntDraw(const HuntMode& h, uint16_t freq, bool ssb, bool usb) {
  if (screen_ != UiScreen::Hunt) {
    return;
  }
  static uint8_t lastRssi = 255;
  static uint8_t lastSnr = 255;
  static uint8_t lastPk = 255;
  static uint16_t lastFreq = 0;
  static bool lastAtPeak = false;
  static bool lastSsb = false;
  static bool lastUsb = false;
  if (!toolChrome_) {
    drawToolTitle("HOMING");
    drawOutlineKey(kSstvBack, "BACK", false);
    drawOutlineKey(kHuntPeak, "PEAK", false);
    tft.fillRect(kHuntPlot.x, kHuntPlot.y, kHuntPlot.w, kHuntPlot.h, theme::bg);
    tft.drawRect(kHuntPlot.x - 1, kHuntPlot.y - 1, kHuntPlot.w + 2, kHuntPlot.h + 2, theme::line);
    tft.fillRect(8, 200, 224, 16, theme::bg);
    drawSpacedLeft("SWEEP", 10, 208, 1, 1, theme::muted);
    drawHuntMeterChrome(220, "RSSI");
    drawHuntMeterChrome(248, "SNR");
    drawBrackets(304, 316);
    drawSpaced("ROTATE ANTENNA", 120, 310, 1, 1, theme::muted);
    lastRssi = 255;
    lastSnr = 255;
    lastPk = 255;
    lastFreq = 0;
    lastAtPeak = false;
    lastSsb = !ssb;
    lastUsb = !usb;
    toolChrome_ = true;
  }

  const uint8_t* hist = h.history();
  const uint8_t head = h.histHead();
  const uint8_t pk = h.peak() > 80 ? 80 : h.peak();
  const int16_t peakY = huntPlotY(pk);

  huntSpr.fillSprite(theme::bg);
  for (uint8_t g = 0; g <= 4; ++g) {
    huntSpr.drawFastHLine(kHuntPlotX, huntPlotY(static_cast<uint8_t>(g * 20)), kHuntHist, theme::dim);
  }

  int16_t prevY = kHuntPlotBot;
  int16_t lastX = kHuntPlotX;
  int16_t lastY = kHuntPlotBot;
  for (uint8_t i = 0; i < kHuntHist; ++i) {
    const uint8_t idx = static_cast<uint8_t>((head + i) % kHuntHist);
    const uint8_t v = hist[idx];
    const int16_t x = static_cast<int16_t>(kHuntPlotX + i);
    const int16_t y = huntPlotY(v);
    if (y < kHuntPlotBot) {
      huntSpr.drawFastVLine(x, y, static_cast<int16_t>(kHuntPlotBot - y), theme::dim);
    }
    if (i > 0) {
      huntSpr.drawLine(static_cast<int16_t>(x - 1), prevY, x, y, theme::sand);
    }
    prevY = y;
    lastX = x;
    lastY = y;
  }

  huntSpr.drawFastHLine(kHuntPlotX, kHuntPlotBot, kHuntHist, theme::muted);
  dashHLine(huntSpr, kHuntPlotX, peakY, kHuntHist, theme::line);
  huntSpr.drawFastHLine(static_cast<int16_t>(kHuntPlotX + kHuntHist - 7), peakY, 7, theme::sand);
  huntSpr.drawFastVLine(static_cast<int16_t>(kHuntPlotX + kHuntHist - 1),
                        static_cast<int16_t>(peakY - 2), 5, theme::sand);

  huntSpr.drawFastVLine(lastX, lastY, static_cast<int16_t>(kHuntPlotBot - lastY), theme::sand);
  huntSpr.fillCircle(lastX, lastY, 2, theme::sand);

  huntSpr.setFreeFont(nullptr);
  huntSpr.setTextFont(1);
  huntSpr.setTextDatum(MR_DATUM);
  huntSpr.setTextColor(theme::muted, theme::bg);
  huntSpr.drawString("80", static_cast<int16_t>(kHuntPlotX - 3), huntPlotY(80), 1);
  huntSpr.drawString("40", static_cast<int16_t>(kHuntPlotX - 3), huntPlotY(40), 1);
  huntSpr.drawString("0", static_cast<int16_t>(kHuntPlotX - 3), huntPlotY(0), 1);
  huntSpr.setTextDatum(ML_DATUM);
  huntSpr.setTextColor(theme::line, theme::bg);
  huntSpr.drawString("PK", static_cast<int16_t>(kHuntPlotX + 2), static_cast<int16_t>(peakY - 6), 1);
  huntSpr.pushSprite(static_cast<int32_t>(kHuntPlot.x + 1), static_cast<int32_t>(kHuntPlot.y + 1));

  const bool atPeak = h.peak() > 6 && h.rssi() + 1 >= h.peak();
  const bool peakStateChanged = lastAtPeak != atPeak;
  if (lastFreq != freq || peakStateChanged || lastSsb != ssb || lastUsb != usb) {
    lastFreq = freq;
    lastAtPeak = atPeak;
    lastSsb = ssb;
    lastUsb = usb;
    tft.fillRect(8, 200, 224, 16, theme::bg);
    drawSpacedLeft(atPeak ? "MAX" : "HOMING", 10, 208, 1, 1, atPeak ? theme::sand : theme::muted);
    char tune[24];
    snprintf(tune, sizeof(tune), "%u  %s", freq, ssb ? (usb ? "USB" : "LSB") : "AM/FM");
    drawSpacedRight(tune, 230, 208, 1, 1, theme::muted);
  }

  if (lastRssi != h.rssi() || lastPk != h.peak() || peakStateChanged) {
    lastRssi = h.rssi();
    lastPk = h.peak();
    char pkTxt[12];
    if (atPeak) {
      snprintf(pkTxt, sizeof(pkTxt), "MAX");
    } else {
      snprintf(pkTxt, sizeof(pkTxt), "PK %02u", h.peak());
    }
    drawHuntMeterData(220, h.rssi(), 80, static_cast<int>(pk), pkTxt,
                      atPeak ? theme::sand : theme::line);
  }
  if (lastSnr != h.snr()) {
    lastSnr = h.snr();
    drawHuntMeterData(248, h.snr(), 40, -1, nullptr, theme::muted);
  }
}

void DisplayUI::cwDraw(const CwDecoder& cw, uint16_t freq, bool usb, int16_t bfo, uint8_t volume,
                      bool mic) {
  if (screen_ != UiScreen::Cw) {
    return;
  }
  static int lastLen = -1;
  static bool lastUsb = false;
  static bool lastMic = true;
  static uint16_t lastPitch = 0;
  static int16_t lastBfo = 0;
  static uint8_t lastVol = 255;
  static uint16_t lastFreq = 0;
  static uint8_t lastWpm = 255;
  static bool lastAudio = true;
  if (!toolChrome_) {
    drawToolTitle("MORSE");
    drawOutlineKey(kSstvBack, "BACK", btnLit(TouchEvent::SstvBack));
    drawOutlineKey(kSstvUsb, usb ? "USB" : "LSB", true);
    tft.fillRect(kCwCopy.x, kCwCopy.y, kCwCopy.w, kCwCopy.h, theme::bg);
    tft.drawRect(kCwCopy.x - 1, kCwCopy.y - 1, kCwCopy.w + 2, kCwCopy.h + 2, theme::line);
    drawOutlineKey(kCwSrc, mic ? "MIC" : "RADIO", mic || btnLit(TouchEvent::CwSrc));
    drawOutlineKey(kCwPitchDn, "PIT-", btnLit(TouchEvent::CwPitchDown));
    drawOutlineKey(kCwPitchUp, "PIT+", btnLit(TouchEvent::CwPitchUp));
    drawOutlineKey(kCwBfoDn, "BFO-", btnLit(TouchEvent::SstvBfoDown));
    drawOutlineKey(kCwBfoUp, "BFO+", btnLit(TouchEvent::SstvBfoUp));
    drawOutlineKey(kCwVolDn, "VOL-", btnLit(TouchEvent::VolDown));
    drawOutlineKey(kCwVolUp, "VOL+", btnLit(TouchEvent::VolUp));
    tft.drawRect(kCwPitchVal.x, kCwPitchVal.y, kCwPitchVal.w, kCwPitchVal.h, theme::dim);
    tft.drawRect(kCwBfoVal.x, kCwBfoVal.y, kCwBfoVal.w, kCwBfoVal.h, theme::dim);
    tft.drawRect(kCwVolVal.x, kCwVolVal.y, kCwVolVal.w, kCwVolVal.h, theme::dim);
    lastLen = -1;
    lastUsb = usb;
    lastMic = mic;
    lastPitch = 0xFFFF;
    lastBfo = INT16_MIN;
    lastVol = 255;
    lastFreq = 0;
    lastWpm = 255;
    lastAudio = !cw.hasAudio();
    toolChrome_ = true;
  }
  cwUsbShown = usb;
  cwMicShown = mic;
  if (lastUsb != usb) {
    drawOutlineKey(kSstvUsb, usb ? "USB" : "LSB", true);
    lastUsb = usb;
  }
  if (lastMic != mic) {
    drawOutlineKey(kCwSrc, mic ? "MIC" : "RADIO", mic || btnLit(TouchEvent::CwSrc));
    lastMic = mic;
    lastFreq = 0;
  }

  const char* t = cw.text();
  const int len = static_cast<int>(strlen(t));
  if (len != lastLen) {
    lastLen = len;
    drawCwCopy(t, static_cast<int16_t>(kCwCopy.x + 4), static_cast<int16_t>(kCwCopy.y + 4),
               static_cast<int16_t>(kCwCopy.w - 8), static_cast<int16_t>(kCwCopy.h - 8));
  }

  if (lastFreq != freq || lastWpm != cw.wpm() || lastAudio != cw.hasAudio()) {
    lastFreq = freq;
    lastWpm = cw.wpm();
    lastAudio = cw.hasAudio();
    char left[16];
    uint16_t leftFg = theme::dim;
    if (cw.hasAudio()) {
      snprintf(left, sizeof(left), "%u WPM", cw.wpm());
      leftFg = theme::sand;
    } else {
      snprintf(left, sizeof(left), "%s", mic ? "MIC" : "LISTEN");
    }
    char tune[16];
    snprintf(tune, sizeof(tune), "%u", freq);
    drawCwStatus(kCwHud, left, leftFg, tune);
  }

  if (lastPitch != cw.pitch()) {
    char p[12];
    snprintf(p, sizeof(p), "%u", cw.pitch());
    drawCwValue(kCwPitchVal, p, theme::sand);
    lastPitch = cw.pitch();
  }
  if (lastBfo != bfo) {
    char b[12];
    snprintf(b, sizeof(b), "%+d", static_cast<int>(bfo));
    drawCwValue(kCwBfoVal, b, theme::muted);
    lastBfo = bfo;
  }
  if (lastVol != volume) {
    char v[12];
    snprintf(v, sizeof(v), "%u", volume);
    drawCwValue(kCwVolVal, v, theme::sand);
    lastVol = volume;
  }
}

void DisplayUI::logDraw(const NsLogger& lg, uint16_t freq, bool usb, bool ssb) {
  if (screen_ != UiScreen::Log) {
    return;
  }
  logArmed_ = lg.armed();
  logUsb_ = usb;
  logSsb_ = ssb;
  logPreset_ = lg.preset();
  static uint32_t lastList = 0xFFFFFFFFu;
  static bool lastArmed = false;
  static bool lastUsb = false;
  static bool lastSsb = false;
  static uint8_t lastPreset = 255;
  static uint8_t lastThr = 255;
  static uint8_t lastCount = 255;
  static bool lastOpen = true;
  static bool lastHudArmed = true;
  static uint16_t lastFreq = 0;
  if (!toolChrome_) {
    drawToolTitle("WATCH");
    drawOutlineKey(kSstvBack, "BACK", btnLit(TouchEvent::SstvBack));
    drawOutlineKey(kSstvUsb, ssb ? (usb ? "USB" : "LSB") : "AM", true);
    tft.fillRect(kLogList.x, kLogList.y, kLogList.w, kLogList.h, theme::bg);
    tft.drawRect(kLogList.x - 1, kLogList.y - 1, kLogList.w + 2, kLogList.h + 2, theme::line);
    drawOutlineKey(kLogFreqDn, "-", btnLit(TouchEvent::TuneDown));
    drawOutlineKey(kLogFreqUp, "+", btnLit(TouchEvent::TuneUp));
    tft.drawRect(kLogFreqVal.x, kLogFreqVal.y, kLogFreqVal.w, kLogFreqVal.h, theme::dim);
    drawOutlineKey(kLogThrDn, "THR-", btnLit(TouchEvent::LogThrDown));
    drawOutlineKey(kLogArm, lg.armed() ? "ARM" : "IDLE", lg.armed() || btnLit(TouchEvent::LogArm));
    drawOutlineKey(kLogClr, "CLR", btnLit(TouchEvent::LogClr));
    drawOutlineKey(kLogThrUp, "THR+", btnLit(TouchEvent::LogThrUp));
    drawLogPresetRow(lg.preset(), btnFocusSlot(TouchEvent::LogPreset));
    lastList = 0xFFFFFFFFu;
    lastArmed = lg.armed();
    lastUsb = usb;
    lastSsb = ssb;
    lastPreset = lg.preset();
    lastThr = 255;
    lastCount = 255;
    lastOpen = !lg.open();
    lastHudArmed = !lg.armed();
    lastFreq = 0;
    toolChrome_ = true;
  }
  if (lastArmed != lg.armed()) {
    drawOutlineKey(kLogArm, lg.armed() ? "ARM" : "IDLE", lg.armed() || btnLit(TouchEvent::LogArm));
    lastArmed = lg.armed();
  }
  if (lastUsb != usb || lastSsb != ssb) {
    drawOutlineKey(kSstvUsb, ssb ? (usb ? "USB" : "LSB") : "AM", true);
    lastUsb = usb;
    lastSsb = ssb;
  }

  const uint8_t n = lg.count();
  uint32_t listKey = (static_cast<uint32_t>(n) << 17) | (lg.open() ? 0x10000u : 0u);
  if (n > 0) {
    listKey |= static_cast<uint32_t>(lg.at(0).peakRssi) << 8;
    listKey |= lg.at(0).peakSnr;
  }
  if (listKey != lastList) {
    lastList = listKey;
    tft.fillRect(static_cast<int16_t>(kLogList.x + 1), static_cast<int16_t>(kLogList.y + 1),
                 static_cast<int16_t>(kLogList.w - 2), static_cast<int16_t>(kLogList.h - 2),
                 theme::bg);
    useBitmapFont(1);
    tft.setTextDatum(TL_DATUM);
    if (n == 0) {
      drawSpacedLeft("NO EVENTS", 14, 58, 2, 2, theme::muted);
      drawSpacedLeft("ARM WATCHES RSSI", 14, 78, 2, 1, theme::dim);
    } else {
      const uint8_t rows = n > 7 ? 7 : n;
      for (uint8_t i = 0; i < rows; ++i) {
        const NsEvent& e = lg.at(i);
        char line[40];
        snprintf(line, sizeof(line), "%s %u %u/%u", e.when, e.freq, e.peakRssi, e.peakSnr);
        const bool live = i == 0 && lg.open();
        tft.setTextColor(live ? theme::sand : theme::muted, theme::bg);
        tft.drawString(line, 16, 44 + static_cast<int>(i) * 20, 1);
        if (live) {
          tft.fillRect(12, 45 + static_cast<int>(i) * 20, 2, 6, theme::sand);
        }
      }
    }
  }

  if (lastFreq != freq || lastThr != lg.threshold() || lastOpen != lg.open() ||
      lastCount != n || lastHudArmed != lg.armed()) {
    lastFreq = freq;
    lastThr = lg.threshold();
    lastOpen = lg.open();
    lastCount = n;
    lastHudArmed = lg.armed();
    tft.fillRect(8, 200, 224, 16, theme::bg);
    const char* st = lg.open() ? "OPEN" : (lg.armed() ? "WATCH" : "IDLE");
    const uint16_t stCol = lg.open() ? theme::sand : (lg.armed() ? theme::line : theme::dim);
    drawSpacedLeft(st, 10, 208, 1, 1, stCol);
    char thr[16];
    snprintf(thr, sizeof(thr), "THR %u", lg.threshold());
    drawSpacedRight(thr, 230, 208, 1, 1, theme::muted);
    char f[12];
    snprintf(f, sizeof(f), "%u", freq);
    drawCwValue(kLogFreqVal, f, theme::sand);
  }

  if (lastPreset != lg.preset()) {
    drawLogPresetRow(lg.preset(), btnFocusSlot(TouchEvent::LogPreset));
    lastPreset = lg.preset();
  }
}

}
