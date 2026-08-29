#pragma once

#include <Arduino.h>
#include "AudioTap.h"

namespace fennec {

enum class SstvSource : uint8_t { Radio = 0, Mic = 1 };

constexpr uint16_t kSstvOutW = 224;
constexpr uint16_t kSstvOutH = 176;
constexpr uint32_t kSstvFs = AdcTap::kFs;

enum class SstvMode : uint8_t {
  Unknown = 0,
  MartinM1,
  MartinM2,
  ScottieS1,
  ScottieS2,
  Robot36,
};

struct SstvLine {
  uint16_t y = 0;
  uint16_t px[kSstvOutW] = {};
};

struct SstvStatus {
  bool running = false;
  bool hasAudio = false;
  bool finished = false;
  SstvMode mode = SstvMode::Unknown;
  uint16_t vis = 0;
  SstvMode prefer = SstvMode::Unknown;
  uint16_t line = 0;
  uint16_t lines = 0;
  uint16_t freqHz = 0;
  uint16_t peak = 0;
};

class SstvDecoder {
 public:
  void begin();
  void start();
  void stop();
  void poll();

  void setSource(SstvSource src);
  bool mic() const { return source_ == SstvSource::Mic; }
  void cyclePreferMode();

  bool takeLine(SstvLine* out);
  SstvStatus status() const { return status_; }
  bool active() const { return running_; }
  const char* modeName() const;
  const uint16_t* frame() const { return frame_; }
  uint32_t frameGen() const { return frameGen_; }

 private:
  enum class Phase : uint8_t { Idle, Hunt, Vis, Image };

  struct ModeSpec {
    SstvMode mode;
    uint16_t vis;
    uint16_t width;
    uint16_t height;
    float syncS;
    float porchS;
    float pixelS;
    float sepS;
    uint8_t channels;
    bool scottie;
    bool robot;
  };

  float demod(int16_t s);
  void processFreq(float hz);
  void enterVis();
  void visBit(bool one);
  bool applyVis(uint8_t vis);
  void applyMode(SstvMode m);
  void enterImage();
  void imageSample(float hz, float dt);
  void emitRgbLine();
  void emitRobotLine();
  void stampFrame();
  static uint8_t toneToGray(float hz);
  static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
  static ModeSpec specFor(SstvMode m);

  bool popSample(int16_t* s);

  SstvSource source_ = SstvSource::Radio;
  SstvMode prefer_ = SstvMode::Unknown;
  bool running_ = false;
  bool inited_ = false;
  uint32_t mixPh_ = 0;
  float iL_ = 0;
  float qL_ = 0;
  float prevI_ = 0;
  float prevQ_ = 0;
  float dc_ = 0;
  float freq_ = 1500;
  float freqSmooth_ = 1500;
  float afc_ = 0;
  float leaderSum_ = 0;
  uint16_t leaderN_ = 0;
  float syncDur_ = 0;

  Phase phase_ = Phase::Idle;
  ModeSpec spec_{};
  float acc_ = 0;
  float hunt1900_ = 0;
  float visTimer_ = 0;
  float visAge_ = 0;
  uint8_t visBits_ = 0;
  uint8_t visCount_ = 0;
  uint8_t visSkip_ = 0;
  uint16_t pix_ = 0;
  uint16_t line_ = 0;
  uint8_t ch_ = 0;
  uint8_t g_[kSstvOutW] = {};
  uint8_t b_[kSstvOutW] = {};
  uint8_t rch_[kSstvOutW] = {};
  uint8_t yPrev_[kSstvOutW] = {};
  uint8_t ryHold_[kSstvOutW] = {};
  uint8_t byHold_[kSstvOutW] = {};

  bool lineReady_ = false;
  SstvLine out_{};
  SstvStatus status_{};
  uint32_t peakAcc_ = 0;
  uint16_t peakN_ = 0;
  uint16_t* frame_ = nullptr;
  uint32_t frameGen_ = 0;
};

}
