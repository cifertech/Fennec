#pragma once

#include <Arduino.h>
#include "Radio.h"

namespace fennec {

constexpr uint8_t kHuntHist = 200;

class HuntMode {
 public:
  void start();
  void stop();
  bool active() const { return active_; }
  void poll(Radio& radio);
  void resetPeak();

  uint8_t rssi() const { return rssi_; }
  uint8_t snr() const { return snr_; }
  uint8_t peak() const { return peak_; }
  const uint8_t* history() const { return hist_; }
  uint8_t histCount() const { return kHuntHist; }
  uint8_t histHead() const { return head_; }

  void pixel(uint8_t* r, uint8_t* g, uint8_t* b) const;

 private:
  bool active_ = false;
  uint8_t rssi_ = 0;
  uint8_t snr_ = 0;
  uint8_t peak_ = 0;
  uint8_t hist_[kHuntHist] = {};
  uint8_t head_ = 0;
  uint32_t lastMs_ = 0;
};

}
