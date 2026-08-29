#pragma once

#include <Arduino.h>
#include "AudioTap.h"

namespace fennec {

enum class CwSource : uint8_t { Radio = 0, Mic = 1 };

class CwDecoder {
 public:
  void begin();
  void start();
  void stop();
  void poll();
  bool active() const { return running_; }

  void setSource(CwSource src);
  bool mic() const { return source_ == CwSource::Mic; }

  void setPitch(uint16_t hz);
  void nudgePitch(int16_t delta);
  uint16_t pitch() const { return pitch_; }

  uint8_t wpm() const { return wpm_; }
  bool hasAudio() const { return hasAudio_; }
  const char* text() const { return text_; }

 private:
  void pushMag(float mag, float dt);
  void noteMark(float onS);
  void trainFromPending();
  void setUnit(float u);
  void flushChar();
  void addChar(char c);
  static char lookup(uint16_t bits, uint8_t n);

  bool popSample(int16_t* s);

  static constexpr uint8_t kPending = 24;

  CwSource source_ = CwSource::Radio;
  bool running_ = false;
  uint16_t pitch_ = 700;
  uint8_t wpm_ = 18;
  bool hasAudio_ = false;
  bool keyed_ = false;
  bool trained_ = false;
  float env_ = 0;
  float noise_ = 1.0f;
  float onS_ = 0;
  float offS_ = 0;
  float unitS_ = 0.060f;
  bool idle_ = true;
  float peakTone_ = 0;
  uint16_t bits_ = 0;
  uint8_t nBits_ = 0;
  float pendingOn_[kPending] = {};
  float pendingGap_[kPending] = {};
  uint8_t pendingN_ = 0;
  char text_[240] = {};
  uint8_t textLen_ = 0;
  float goCoef_ = 0;
  float q0_ = 0;
  float q1_ = 0;
  uint16_t blk_ = 0;
  uint32_t peakN_ = 0;
  uint32_t peakAcc_ = 0;
};

}
