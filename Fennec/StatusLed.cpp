#include "Periph.h"

#include "Config.h"
#include "Pins.h"

namespace fennec {

namespace {
constexpr uint16_t kPixelBits = 24 * FENNEC_NEOPIXEL_COUNT;
rmt_data_t ledData_[kPixelBits];
rmt_obj_t* rmt_ = nullptr;

void encodeByte(rmt_data_t*& dst, uint8_t v) {
  for (int bit = 7; bit >= 0; --bit) {
    if (v & (1 << bit)) {
      dst->level0 = 1;
      dst->duration0 = 8;
      dst->level1 = 0;
      dst->duration1 = 4;
    } else {
      dst->level0 = 1;
      dst->duration0 = 4;
      dst->level1 = 0;
      dst->duration1 = 8;
    }
    ++dst;
  }
}

bool ensureRmt() {
  if (rmt_ != nullptr) {
    return true;
  }
  pinMode(PIN_NEOPIXEL, OUTPUT);
  digitalWrite(PIN_NEOPIXEL, LOW);
  delayMicroseconds(300);
  rmt_ = rmtInit(PIN_NEOPIXEL, RMT_TX_MODE, RMT_MEM_128);
  if (rmt_ == nullptr) {
    return false;
  }
  rmtSetTick(rmt_, 100);
  return true;
}
}

void StatusLed::write(uint8_t r, uint8_t g, uint8_t b) {
  if (!ensureRmt()) {
    return;
  }
  uint16_t level = r;
  if (g > level) {
    level = g;
  }
  if (b > level) {
    level = b;
  }
  level = (level * FENNEC_NEOPIXEL_BRIGHT) / 255;
  const uint8_t rs = static_cast<uint8_t>(level);
  const uint8_t gs = static_cast<uint8_t>((110 * level) / 255);
  const uint8_t bs = 0;

#if FENNEC_NEOPIXEL_ORDER_GRB
  const uint8_t order[3] = {gs, rs, bs};
#else
  const uint8_t order[3] = {rs, gs, bs};
#endif

  rmt_data_t* dst = ledData_;
  for (uint16_t i = 0; i < FENNEC_NEOPIXEL_COUNT; ++i) {
    encodeByte(dst, order[0]);
    encodeByte(dst, order[1]);
    encodeByte(dst, order[2]);
  }
  rmtWriteBlocking(rmt_, ledData_, kPixelBits);
  delayMicroseconds(60);
}

void StatusLed::begin() {
  if (!ensureRmt()) {
    Serial.println(F("[pixel] RMT init failed"));
    return;
  }

  write(0, 0, 0);
  delay(2);

  write(255, 72, 0);
  delay(180);
  write(0, 0, 0);

  ready_ = true;
  lastR_ = 0;
  lastG_ = 0;
  lastB_ = 0;
}

void StatusLed::setRgb(uint8_t r, uint8_t g, uint8_t b) {
  if (!ready_ || !enabled_) {
    return;
  }
  if (r == lastR_ && g == lastG_ && b == lastB_) {
    return;
  }
  lastR_ = r;
  lastG_ = g;
  lastB_ = b;
  write(r, g, b);
}

void StatusLed::setEnabled(bool on) {
  enabled_ = on;
  if (!on && ready_) {
    lastR_ = 0;
    lastG_ = 0;
    lastB_ = 0;
    write(0, 0, 0);
  }
}

}
