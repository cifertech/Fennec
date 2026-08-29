// Fennec - copy this file OVER:
//   Arduino/libraries/TFT_eSPI/User_Setup.h
//
// TFT SPI only. XPT2046 touch uses a second SPI bus (see Pins.h).

#define USER_SETUP_INFO "Fennec_ILI9341_ESP32S3"

#define ILI9341_DRIVER

#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   14
#define TFT_DC   21
#define TFT_RST  47
#define TFT_BL   48
#define TFT_BACKLIGHT_ON HIGH

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_GFXFF

#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  20000000
