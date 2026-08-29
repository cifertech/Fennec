#pragma once

// Fennec firmware tunables. Hardware pins live in Pins.h.

#ifndef FENNEC_VERSION
#define FENNEC_VERSION "1.0.0"
#endif

// Set to 1 to run the UI / web stack without an SI4732 attached.
#ifndef FENNEC_SIMULATE_RADIO
#define FENNEC_SIMULATE_RADIO 0
#endif

// ITU Region 1 medium-wave channel spacing is 9 kHz (531-1602).
// Americas MW is typically 10 kHz (530-1700) - flip this if needed.
// The MW chip spans 520-1710 kHz so both plans fit; this only sets the step.
#ifndef FENNEC_MW_STEP_KHZ
#define FENNEC_MW_STEP_KHZ 9
#endif

#ifndef FENNEC_DEFAULT_VOLUME
#define FENNEC_DEFAULT_VOLUME 40  // SI4732 range 0-63
#endif

#ifndef FENNEC_SIGNAL_POLL_MS
#define FENNEC_SIGNAL_POLL_MS 250
#endif

#ifndef FENNEC_RDS_POLL_MS
#define FENNEC_RDS_POLL_MS 800
#endif

#ifndef FENNEC_AMP_UNMUTE_DELAY_MS
#define FENNEC_AMP_UNMUTE_DELAY_MS 80
#endif

#ifndef FENNEC_BAND_SWITCH_MUTE_MS
#define FENNEC_BAND_SWITCH_MUTE_MS 40
#endif

#ifndef FENNEC_BTN_DEBOUNCE_MS
#define FENNEC_BTN_DEBOUNCE_MS 35
#endif

#ifndef FENNEC_LONG_PRESS_MS
#define FENNEC_LONG_PRESS_MS 700
#endif

#ifndef FENNEC_I2C_HZ
#define FENNEC_I2C_HZ 400000
#endif

#ifndef FENNEC_WIFI_AP_SSID
#define FENNEC_WIFI_AP_SSID "Fennec"
#endif

#ifndef FENNEC_WIFI_AP_PASS
#define FENNEC_WIFI_AP_PASS ""  // open AP; set a password (8+ chars) to encrypt
#endif

#ifndef FENNEC_WIFI_AP_CHANNEL
#define FENNEC_WIFI_AP_CHANNEL 1
#endif

#ifndef FENNEC_HOSTNAME
#define FENNEC_HOSTNAME "fennec"
#endif

// Soft-AP is always started so the radio is reachable without a home network.
#ifndef FENNEC_WIFI_ENABLE_AP
#define FENNEC_WIFI_ENABLE_AP 1
#endif

// Optional home-network join. Leave SSID empty to skip STA mode.
#ifndef FENNEC_WIFI_STA_SSID
#define FENNEC_WIFI_STA_SSID ""
#endif

#ifndef FENNEC_WIFI_STA_PASS
#define FENNEC_WIFI_STA_PASS ""
#endif

// POSIX TZ. UTC is the SWL default. Iran ST: "IRST-3:30"
#ifndef FENNEC_TZ
#define FENNEC_TZ "UTC0"
#endif

#ifndef FENNEC_NTP_SERVER
#define FENNEC_NTP_SERVER "pool.ntp.org"
#endif

#ifndef FENNEC_TFT_ROTATION
#define FENNEC_TFT_ROTATION 2  // portrait, 240x320 (180 deg from native)
#endif

#ifndef FENNEC_TFT_INVERT
#define FENNEC_TFT_INVERT 0
#endif

// ILI9341 backlight PWM on PIN_TFT_BL. Percent, never 0 so Settings stays readable.
#ifndef FENNEC_DEFAULT_BRIGHTNESS
#define FENNEC_DEFAULT_BRIGHTNESS 100
#endif
#ifndef FENNEC_BRIGHTNESS_MIN
#define FENNEC_BRIGHTNESS_MIN 20
#endif
#ifndef FENNEC_BRIGHTNESS_STEP
#define FENNEC_BRIGHTNESS_STEP 20
#endif

// XPT2046 raw ADC range (0-4095). CAL_4 flags: 1=swap XY, 2=invert X, 4=invert Y.
#ifndef FENNEC_TOUCH_CAL_0
#define FENNEC_TOUCH_CAL_0 200
#define FENNEC_TOUCH_CAL_1 3900
#define FENNEC_TOUCH_CAL_2 200
#define FENNEC_TOUCH_CAL_3 3900
#define FENNEC_TOUCH_CAL_4 0
#endif

#ifndef FENNEC_TOUCH_Z_MIN
#define FENNEC_TOUCH_Z_MIN 200
#endif

// 1 = print raw X/Y/Z on Serial when the panel is pressed
#ifndef FENNEC_TOUCH_DEBUG
#define FENNEC_TOUCH_DEBUG 0
#endif
#ifndef FENNEC_TOUCH_REPEAT_MS
#define FENNEC_TOUCH_REPEAT_MS 180
#endif

// WS2812 on GPIO 38. Brightness 0-255. If red/green are swapped, set ORDER_GRB to 0.
#ifndef FENNEC_NEOPIXEL_BRIGHT
#define FENNEC_NEOPIXEL_BRIGHT 48
#endif
#ifndef FENNEC_NEOPIXEL_ORDER_GRB
#define FENNEC_NEOPIXEL_ORDER_GRB 1
#endif

#ifndef FENNEC_SSTV_DEFAULT_KHZ
#define FENNEC_SSTV_DEFAULT_KHZ 14230  // 20 m USB calling
#endif
#ifndef FENNEC_SSTV_BFO_STEP
#define FENNEC_SSTV_BFO_STEP 20
#endif

// I2S MEMS mic (GPIO 40/41/42). Right-shift of 32-bit samples into int16.
#ifndef FENNEC_MIC_SHIFT
#define FENNEC_MIC_SHIFT 14
#endif
#ifndef FENNEC_MIC_SLOT_RIGHT
#define FENNEC_MIC_SLOT_RIGHT 0
#endif
