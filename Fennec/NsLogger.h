#pragma once

#include <Arduino.h>
#include "Radio.h"

namespace fennec {

struct NsPreset {
  const char* name;
  uint16_t khz;
  bool usb;
  bool am;
};

struct NsEvent {
  char when[20];
  uint16_t freq;
  bool usb;
  bool am;
  uint8_t peakRssi;
  uint8_t peakSnr;
  char cw[20];
};

constexpr uint8_t kNsLogCap = 32;
constexpr uint8_t kNsPresetCount = 5;

class NsLogger {
 public:
  static const NsPreset* presets();

  void begin();
  void start();
  void stop();
  bool active() const { return active_; }

  void poll(Radio& radio, const char* nowIso, const char* cwTail);

  void setArmed(bool on);
  bool armed() const { return armed_; }
  void clear();
  void setThreshold(uint8_t t);
  void nudgeThreshold(int8_t d);
  uint8_t threshold() const { return thr_; }

  uint8_t preset() const { return preset_; }
  void setPreset(uint8_t i);
  void applyPreset(Radio& radio);

  uint8_t count() const { return count_; }
  const NsEvent& at(uint8_t i) const;
  bool open() const { return open_; }

  void dumpJson(char* out, size_t n) const;
  void dumpSerial() const;

 private:
  void load();
  void save();
  void pushEvent(const NsEvent& e);

  bool active_ = false;
  bool armed_ = false;
  bool open_ = false;
  uint8_t thr_ = 25;
  uint8_t preset_ = kNsPresetCount - 1;
  uint32_t aboveSince_ = 0;
  uint32_t belowSince_ = 0;
  uint8_t peakRssi_ = 0;
  uint8_t peakSnr_ = 0;
  uint32_t lastMs_ = 0;
  NsEvent cur_{};
  NsEvent log_[kNsLogCap];
  uint8_t count_ = 0;
  uint8_t head_ = 0;
};

}
