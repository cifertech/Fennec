#include "HuntMode.h"

namespace fennec {

void HuntMode::start() {
  rssi_ = 0;
  snr_ = 0;
  peak_ = 0;
  head_ = 0;
  for (uint8_t i = 0; i < kHuntHist; ++i) {
    hist_[i] = 0;
  }
  lastMs_ = 0;
  active_ = true;
  Serial.println(F("[homing] RSSI sweep - rotate antenna, watch peak"));
}

void HuntMode::stop() { active_ = false; }

void HuntMode::resetPeak() { peak_ = rssi_; }

void HuntMode::poll(Radio& radio) {
  if (!active_) {
    return;
  }
  const uint32_t now = millis();
  if (now - lastMs_ < 40) {
    return;
  }
  lastMs_ = now;
  const SignalQuality q = radio.readSignal();
  rssi_ = q.rssi;
  snr_ = q.snr;
  if (rssi_ > peak_) {
    peak_ = rssi_;
  }
  hist_[head_] = rssi_ > 80 ? 80 : rssi_;
  head_ = static_cast<uint8_t>((head_ + 1) % kHuntHist);
}

void HuntMode::pixel(uint8_t* r, uint8_t* g, uint8_t* b) const {
  if (r == nullptr || g == nullptr || b == nullptr) {
    return;
  }
  const uint8_t v = rssi_ > 70 ? 70 : rssi_;
  const uint16_t period = static_cast<uint16_t>(900 - (static_cast<uint16_t>(v) * 11));
  const uint32_t now = millis();
  const bool on = (now % (period < 80 ? 80 : period)) < (period / 3 + 20);
  const uint16_t scale = static_cast<uint16_t>(30 + v * 3);
  if (!on) {
    *r = 18;
    *g = 16;
    *b = 12;
    return;
  }
  *r = static_cast<uint8_t>((231 * scale) / 255);
  *g = static_cast<uint8_t>((201 * scale) / 255);
  *b = static_cast<uint8_t>((138 * scale) / 255);
}

}
