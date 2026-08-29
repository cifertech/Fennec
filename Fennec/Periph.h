#pragma once

#include <Arduino.h>
#include <time.h>
#include "Config.h"

namespace fennec {

enum class InputEvent : uint8_t {
  None = 0,
  TuneUp,
  TuneDown,
  BandNext,
  BandPrev,
  SeekUp,
  SeekDown,
  VolUp,
  VolDown,
  MuteToggle,
  Home,
};

class Input {
 public:
  void begin();
  InputEvent poll();

 private:
  InputEvent pollOneButton(uint8_t index);

  struct Btn {
    uint8_t pin;
    InputEvent shortEvt;
    InputEvent longEvt;
    bool down;
    bool longFired;
    uint32_t tDown;
  };

  Btn btns_[3];
  bool chordDown_ = false;
  bool chordFired_ = false;
  uint32_t chordAt_ = 0;
};

class AudioCtrl {
 public:
  void begin();
  void enableAmp(bool on);
  bool ampEnabled() const { return enabled_; }

 private:
  void writePin(bool enabled);
  bool enabled_ = false;
};

class StatusLed {
 public:
  void begin();
  void setRgb(uint8_t r, uint8_t g, uint8_t b);
  void setEnabled(bool on);

 private:
  void write(uint8_t r, uint8_t g, uint8_t b);
  uint8_t lastR_ = 255;
  uint8_t lastG_ = 255;
  uint8_t lastB_ = 255;
  bool ready_ = false;
  bool enabled_ = true;
};

class ClockAlarm {
 public:
  void begin() {
    setenv("TZ", FENNEC_TZ, 1);
    tzset();
  }

  void onStaUp() {
    if (synced_) {
      return;
    }
    configTime(0, 0, FENNEC_NTP_SERVER);
    ntpStarted_ = true;
    Serial.println(F("[clock] NTP request sent"));
  }

  void poll() {
    if (!ntpStarted_ || synced_) {
      return;
    }
    const time_t now = time(nullptr);
    if (now > 1700000000) {
      synced_ = true;
      Serial.println(F("[clock] NTP sync ok"));
    }
  }

  bool synced() const { return synced_; }

  void format(char* out, size_t n) const {
    if (!synced_ || out == nullptr || n < 6) {
      if (out && n) {
        out[0] = 0;
      }
      return;
    }
    const time_t now = time(nullptr);
    if (now < 1700000000) {
      out[0] = 0;
      return;
    }
    struct tm t = {};
    localtime_r(&now, &t);
    snprintf(out, n, "%02d:%02d", t.tm_hour, t.tm_min);
  }

  void formatIso(char* out, size_t n) const {
    if (out == nullptr || n < 8) {
      return;
    }
    const time_t now = time(nullptr);
    if (synced_ && now > 1700000000) {
      struct tm t = {};
      localtime_r(&now, &t);
      snprintf(out, n, "%04d-%02d-%02d %02d:%02d:%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
               t.tm_hour, t.tm_min, t.tm_sec);
      return;
    }
    snprintf(out, n, "T+%lu", static_cast<unsigned long>(millis() / 1000));
  }

 private:
  bool ntpStarted_ = false;
  bool synced_ = false;
};

}
