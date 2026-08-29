#pragma once

#include <Arduino.h>

namespace fennec {

class AdcTap {
 public:
  static constexpr uint32_t kFs = 8000;
  static constexpr uint16_t kRing = 4096;

  static AdcTap& instance();

  void begin();
  bool start();
  void stop();

  bool pop(int16_t* s);

 private:
  AdcTap() = default;
  static void sampleCb(void* arg);
  void push(int16_t s);

  bool inited_ = false;
  bool running_ = false;
  void* timer_ = nullptr;
  int16_t ring_[kRing] = {};
  volatile uint16_t w_ = 0;
  volatile uint16_t r_ = 0;
  volatile int32_t dc_ = 2048;
};

class MicTap {
 public:
  static constexpr uint32_t kHwFs = 16000;
  static constexpr uint32_t kFs = 8000;
  static constexpr uint16_t kRing = 4096;

  static MicTap& instance();

  void begin();
  bool start();
  void stop();

  void poll();
  bool pop(int16_t* s);

 private:
  MicTap() = default;
  void push(int16_t s);

  bool inited_ = false;
  bool running_ = false;
  void* rx_ = nullptr;
  int16_t ring_[kRing] = {};
  volatile uint16_t w_ = 0;
  volatile uint16_t r_ = 0;
  int16_t hold_ = 0;
  bool haveHold_ = false;
};

}
