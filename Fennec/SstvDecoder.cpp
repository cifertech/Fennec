#include "SstvDecoder.h"

#include <math.h>
#include <string.h>
#include <esp_heap_caps.h>

#include "AudioTap.h"
#include "Config.h"
#include "Pins.h"

namespace fennec {

namespace {
constexpr float kPi = 3.14159265f;
constexpr float kDt = 1.0f / static_cast<float>(kSstvFs);
constexpr uint32_t kMixInc =
    static_cast<uint32_t>(1900.0 * 4294967296.0 / static_cast<double>(kSstvFs));

int16_t kCos[256];
int16_t kSin[256];
bool kLutReady = false;

void ensureLut() {
  if (kLutReady) {
    return;
  }
  for (int i = 0; i < 256; ++i) {
    const float a = 2.0f * kPi * static_cast<float>(i) / 256.0f;
    kCos[i] = static_cast<int16_t>(cosf(a) * 32767.0f);
    kSin[i] = static_cast<int16_t>(sinf(a) * 32767.0f);
  }
  kLutReady = true;
}
}

void SstvDecoder::begin() {
  if (inited_) {
    return;
  }
  ensureLut();
  AdcTap::instance().begin();
  MicTap::instance().begin();
  if (frame_ == nullptr) {
    const size_t bytes = sizeof(uint16_t) * kSstvOutW * kSstvOutH;
    frame_ = static_cast<uint16_t*>(
        heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (frame_ == nullptr) {
      frame_ = static_cast<uint16_t*>(malloc(bytes));
    }
    if (frame_ != nullptr) {
      memset(frame_, 0, bytes);
    } else {
      Serial.println(F("[sstv] no RAM for web frame"));
    }
  }
  inited_ = true;
  Serial.printf("[sstv] ADC GPIO %u @ %u Hz  (SI4732 LOUT -> cap -> mid-bias -> pin)\n", PIN_SSTV_ADC,
                kSstvFs);
  Serial.println(F("[sstv] I2S mic RX / MIC toggle on the SSTV page"));
}

void SstvDecoder::setSource(SstvSource src) {
  if (source_ == src) {
    return;
  }
  const bool live = running_;
  source_ = src;
  if (live) {
    start();
  }
}

namespace {
const char* labelOf(SstvMode m) {
  switch (m) {
    case SstvMode::MartinM1:
      return "Martin M1";
    case SstvMode::MartinM2:
      return "Martin M2";
    case SstvMode::ScottieS1:
      return "Scottie S1";
    case SstvMode::ScottieS2:
      return "Scottie S2";
    case SstvMode::Robot36:
      return "Robot 36";
    default:
      return "AUTO";
  }
}
}

void SstvDecoder::cyclePreferMode() {
  switch (prefer_) {
    case SstvMode::Unknown:
      prefer_ = SstvMode::MartinM1;
      break;
    case SstvMode::MartinM1:
      prefer_ = SstvMode::MartinM2;
      break;
    case SstvMode::MartinM2:
      prefer_ = SstvMode::ScottieS1;
      break;
    case SstvMode::ScottieS1:
      prefer_ = SstvMode::ScottieS2;
      break;
    case SstvMode::ScottieS2:
      prefer_ = SstvMode::Robot36;
      break;
    default:
      prefer_ = SstvMode::Unknown;
      break;
  }
  status_.prefer = prefer_;
  if (status_.mode == SstvMode::Unknown || phase_ == Phase::Hunt || phase_ == Phase::Idle) {
    status_.mode = prefer_;
  }
  Serial.printf("[sstv] mode lock %s\n", labelOf(prefer_));
}

void SstvDecoder::start() {
  begin();
  stop();
  mixPh_ = 0;
  iL_ = qL_ = prevI_ = prevQ_ = dc_ = 0;
  freq_ = freqSmooth_ = 1500;
  afc_ = 0;
  leaderSum_ = 0;
  leaderN_ = 0;
  syncDur_ = 0;
  phase_ = Phase::Hunt;
  acc_ = 0;
  hunt1900_ = 0;
  visTimer_ = 0;
  visAge_ = 0;
  visBits_ = visCount_ = visSkip_ = 0;
  pix_ = line_ = 0;
  ch_ = 0;
  lineReady_ = false;
  if (frame_ != nullptr) {
    memset(frame_, 0, sizeof(uint16_t) * kSstvOutW * kSstvOutH);
    ++frameGen_;
  }
  status_ = {};
  status_.running = true;
  status_.prefer = prefer_;
  status_.mode = prefer_;
  running_ = true;
  bool ok = false;
  if (source_ == SstvSource::Mic) {
    ok = MicTap::instance().start();
  } else {
    ok = AdcTap::instance().start();
  }
  if (!ok) {
    running_ = false;
    status_.running = false;
    return;
  }
  Serial.println(source_ == SstvSource::Mic ? F("[sstv] listening on MIC")
                                           : F("[sstv] listening for VIS / Martin / Scottie / Robot36"));
}

void SstvDecoder::stop() {
  running_ = false;
  AdcTap::instance().stop();
  MicTap::instance().stop();
  status_.running = false;
  phase_ = Phase::Idle;
}

bool SstvDecoder::popSample(int16_t* s) {
  if (source_ == SstvSource::Mic) {
    return MicTap::instance().pop(s);
  }
  return AdcTap::instance().pop(s);
}

float SstvDecoder::demod(int16_t s) {
  float x = static_cast<float>(s);
  dc_ = dc_ * 0.995f + x * 0.005f;
  x -= dc_;
  const uint8_t idx = static_cast<uint8_t>(mixPh_ >> 24);
  mixPh_ += kMixInc;
  const float i = x * static_cast<float>(kCos[idx]);
  const float q = x * static_cast<float>(-kSin[idx]);
  iL_ = iL_ * 0.90f + i * 0.10f;
  qL_ = qL_ * 0.90f + q * 0.10f;
  const float det = prevI_ * qL_ - prevQ_ * iL_;
  const float mag = prevI_ * iL_ + prevQ_ * qL_;
  prevI_ = iL_;
  prevQ_ = qL_;
  const float dph = atan2f(det, mag + 1e-3f);
  freq_ = 1900.0f + dph * (static_cast<float>(kSstvFs) / (2.0f * kPi));
  if (freq_ < 700.0f) {
    freq_ = 700.0f;
  }
  if (freq_ > 2800.0f) {
    freq_ = 2800.0f;
  }
  freqSmooth_ = freqSmooth_ * 0.92f + freq_ * 0.08f;
  return freq_;
}

void SstvDecoder::poll() {
  if (!running_) {
    return;
  }
  int16_t s = 0;
  uint16_t n = 0;
  uint32_t peak = 0;
  const uint32_t t0 = millis();
  if (source_ == SstvSource::Mic) {
    MicTap::instance().poll();
  }
  while (n < 800 && popSample(&s)) {
    const uint16_t a = static_cast<uint16_t>(s < 0 ? -s : s);
    if (a > peak) {
      peak = a;
    }
    processFreq(demod(s));
    ++n;
    if ((n & 127) == 0 && (millis() - t0) >= 18) {
      break;
    }
  }
  yield();
  if (n > 0) {
    peakAcc_ += peak;
    ++peakN_;
    if (peakN_ >= 24) {
      status_.peak = static_cast<uint16_t>(peakAcc_ / peakN_);
      status_.hasAudio = status_.peak > 40;
      peakAcc_ = 0;
      peakN_ = 0;
    }
  }
  status_.freqHz = static_cast<uint16_t>(freqSmooth_ + 0.5f);
  status_.line = line_;
  status_.running = running_;
  status_.prefer = prefer_;
}

void SstvDecoder::processFreq(float hz) {
  (void)hz;
  const float sm = freqSmooth_;
  const float corr = sm - afc_;
  switch (phase_) {
    case Phase::Idle:
      break;
    case Phase::Hunt:
      if (sm > 1600.0f && sm < 2300.0f) {
        hunt1900_ += kDt;
        if (hunt1900_ > 0.50f) {
          hunt1900_ = 0.50f;
        }
        leaderSum_ += sm;
        if (leaderN_ < 60000) {
          ++leaderN_;
        }
        visTimer_ = 0;
        if (leaderN_ > 80) {
          afc_ = (leaderSum_ / static_cast<float>(leaderN_)) - 1900.0f;
        }
        if (hunt1900_ > 0.30f) {
          if (prefer_ != SstvMode::Unknown) {
            applyMode(prefer_);
          } else {
            enterVis();
          }
        }
      } else if (hunt1900_ > 0.12f && corr < 1450.0f) {
        visTimer_ += kDt;
        if (visTimer_ > 0.015f) {
          if (leaderN_ > 40) {
            afc_ = (leaderSum_ / static_cast<float>(leaderN_)) - 1900.0f;
          }
          if (prefer_ != SstvMode::Unknown) {
            applyMode(prefer_);
          } else {
            enterVis();
          }
        }
      } else {
        hunt1900_ -= kDt * 0.20f;
        if (hunt1900_ < 0.0f) {
          hunt1900_ = 0.0f;
          leaderSum_ = 0;
          leaderN_ = 0;
        }
      }
      break;
    case Phase::Vis: {
      visAge_ += kDt;
      visTimer_ += kDt;
      if (visAge_ > 0.45f) {
        if (visCount_ >= 7) {
          applyVis(visBits_ & 0x7F);
        } else {
          applyMode(prefer_ != SstvMode::Unknown ? prefer_ : SstvMode::MartinM1);
        }
        break;
      }
      if (visSkip_ == 0) {
        if (visTimer_ < 0.030f) {
          break;
        }
        visTimer_ = 0;
        visSkip_ = 1;
        break;
      }
      if (visTimer_ < 0.030f) {
        acc_ += corr;
        ++pix_;
        break;
      }
      const float mean = pix_ > 0 ? acc_ / static_cast<float>(pix_) : corr;
      acc_ = 0;
      pix_ = 0;
      visTimer_ = 0;
      if (visCount_ < 8) {
        visBit(mean < 1200.0f);
      } else {
        applyVis(visBits_ & 0x7F);
      }
      break;
    }
    case Phase::Image:
      imageSample(freqSmooth_ - afc_, kDt);
      break;
  }
}

void SstvDecoder::enterVis() {
  phase_ = Phase::Vis;
  visTimer_ = 0;
  visAge_ = 0;
  visBits_ = 0;
  visCount_ = 0;
  visSkip_ = 0;
  acc_ = 0;
  pix_ = 0;
  status_.mode = SstvMode::Unknown;
  Serial.println(F("[sstv] leader - reading VIS"));
}

void SstvDecoder::visBit(bool one) {
  if (one) {
    visBits_ |= static_cast<uint8_t>(1u << visCount_);
  }
  ++visCount_;
}

bool SstvDecoder::applyVis(uint8_t vis) {
  status_.vis = vis;
  SstvMode m = SstvMode::Unknown;
  switch (vis) {
    case 44:
      m = SstvMode::MartinM1;
      break;
    case 40:
      m = SstvMode::MartinM2;
      break;
    case 60:
      m = SstvMode::ScottieS1;
      break;
    case 56:
      m = SstvMode::ScottieS2;
      break;
    case 8:
      m = SstvMode::Robot36;
      break;
    default:
      m = prefer_ != SstvMode::Unknown ? prefer_ : SstvMode::MartinM1;
      Serial.printf("[sstv] VIS %u unknown - %s\n", vis, labelOf(m));
      break;
  }
  applyMode(m);
  status_.vis = vis;
  return true;
}

void SstvDecoder::applyMode(SstvMode m) {
  spec_ = specFor(m);
  status_.mode = spec_.mode;
  status_.vis = spec_.vis;
  status_.lines = spec_.height;
  status_.finished = false;
  status_.prefer = prefer_;
  Serial.printf("[sstv] %s  VIS %u  %ux%u\n", modeName(), spec_.vis, spec_.width, spec_.height);
  enterImage();
}

void SstvDecoder::enterImage() {
  phase_ = Phase::Image;
  acc_ = 0;
  pix_ = 0;
  line_ = 0;
  ch_ = 0;
  syncDur_ = 0;
  acc_ = -(spec_.syncS + spec_.porchS);
}

void SstvDecoder::imageSample(float hz, float dt) {
  if (spec_.mode == SstvMode::Unknown) {
    return;
  }
  acc_ += dt;
  if (acc_ < 0.0f) {
    return;
  }
  const float chanS = spec_.robot ? (ch_ == 0 ? 0.088f : 0.044f)
                                  : spec_.pixelS * static_cast<float>(spec_.width);
  if (acc_ >= chanS) {
    acc_ -= chanS;
    ++ch_;
    pix_ = 0;
    if (spec_.robot) {
      if (ch_ >= 2) {
        emitRobotLine();
        ch_ = 0;
        acc_ -= spec_.syncS + spec_.porchS;
        if (line_ >= spec_.height) {
          status_.finished = true;
          phase_ = Phase::Hunt;
          hunt1900_ = 0;
          Serial.println(F("[sstv] frame complete"));
        }
      } else {
        acc_ -= spec_.sepS;
      }
      return;
    }
    acc_ -= spec_.sepS;
    if (ch_ >= spec_.channels) {
      emitRgbLine();
      ch_ = 0;
      acc_ -= spec_.syncS + spec_.porchS;
      if (line_ >= spec_.height) {
        status_.finished = true;
        phase_ = Phase::Hunt;
        hunt1900_ = 0;
        Serial.println(F("[sstv] frame complete"));
      }
    }
    return;
  }

  const float pixT = spec_.robot ? (ch_ == 0 ? 0.000275f : 0.0001375f) : spec_.pixelS;
  uint16_t p = static_cast<uint16_t>(acc_ / pixT);
  if (p >= spec_.width) {
    p = spec_.width - 1;
  }
  const uint16_t o = static_cast<uint16_t>((static_cast<uint32_t>(p) * kSstvOutW) / spec_.width);
  const uint8_t g = toneToGray(hz);
  if (spec_.robot) {
    if (ch_ == 0) {
      g_[o] = g;
    } else if ((line_ & 1) == 0) {
      ryHold_[o] = g;
    } else {
      byHold_[o] = g;
    }
    return;
  }
  if (ch_ == 0) {
    g_[o] = g;
  } else if (ch_ == 1) {
    b_[o] = g;
  } else {
    rch_[o] = g;
  }
}

void SstvDecoder::emitRgbLine() {
  const uint16_t y = static_cast<uint16_t>((static_cast<uint32_t>(line_) * kSstvOutH) / spec_.height);
  out_.y = y;
  for (uint16_t x = 0; x < kSstvOutW; ++x) {
    out_.px[x] = rgb565(rch_[x], g_[x], b_[x]);
  }
  stampFrame();
  lineReady_ = true;
  ++line_;
}

void SstvDecoder::emitRobotLine() {
  const uint16_t y = static_cast<uint16_t>((static_cast<uint32_t>(line_) * kSstvOutH) / spec_.height);
  out_.y = y;
  for (uint16_t x = 0; x < kSstvOutW; ++x) {
    const int Y = static_cast<int>(g_[x]);
    const int cr = static_cast<int>(ryHold_[x]) - 128;
    const int cb = static_cast<int>(byHold_[x]) - 128;
    int R = Y + cr;
    int B = Y + cb;
    int G = Y - cr / 2 - cb / 4;
    if (R < 0) {
      R = 0;
    }
    if (R > 255) {
      R = 255;
    }
    if (G < 0) {
      G = 0;
    }
    if (G > 255) {
      G = 255;
    }
    if (B < 0) {
      B = 0;
    }
    if (B > 255) {
      B = 255;
    }
    out_.px[x] = rgb565(static_cast<uint8_t>(R), static_cast<uint8_t>(G), static_cast<uint8_t>(B));
    yPrev_[x] = g_[x];
  }
  stampFrame();
  lineReady_ = true;
  ++line_;
}

void SstvDecoder::stampFrame() {
  if (frame_ == nullptr || out_.y >= kSstvOutH) {
    return;
  }
  memcpy(&frame_[static_cast<uint32_t>(out_.y) * kSstvOutW], out_.px, sizeof(out_.px));
  ++frameGen_;
}

bool SstvDecoder::takeLine(SstvLine* out) {
  if (!lineReady_ || out == nullptr) {
    return false;
  }
  *out = out_;
  lineReady_ = false;
  return true;
}

const char* SstvDecoder::modeName() const {
  switch (status_.mode) {
    case SstvMode::MartinM1:
      return "Martin M1";
    case SstvMode::MartinM2:
      return "Martin M2";
    case SstvMode::ScottieS1:
      return "Scottie S1";
    case SstvMode::ScottieS2:
      return "Scottie S2";
    case SstvMode::Robot36:
      return "Robot 36";
    default:
      return "Listening";
  }
}

uint8_t SstvDecoder::toneToGray(float hz) {
  float v = (hz - 1500.0f) / 800.0f;
  if (v < 0.0f) {
    v = 0.0f;
  }
  if (v > 1.0f) {
    v = 1.0f;
  }
  return static_cast<uint8_t>(v * 255.0f + 0.5f);
}

uint16_t SstvDecoder::rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

SstvDecoder::ModeSpec SstvDecoder::specFor(SstvMode m) {
  ModeSpec s{};
  s.mode = m;
  s.width = 320;
  s.channels = 3;
  s.scottie = false;
  s.robot = false;
  switch (m) {
    case SstvMode::MartinM1:
      s.vis = 44;
      s.height = 256;
      s.syncS = 0.004862f;
      s.porchS = 0.000572f;
      s.pixelS = 0.0004576f;
      s.sepS = 0.000572f;
      break;
    case SstvMode::MartinM2:
      s.vis = 40;
      s.height = 256;
      s.syncS = 0.004862f;
      s.porchS = 0.000572f;
      s.pixelS = 0.0002288f;
      s.sepS = 0.000572f;
      break;
    case SstvMode::ScottieS1:
      s.vis = 60;
      s.height = 256;
      s.syncS = 0.009f;
      s.porchS = 0.0015f;
      s.pixelS = 0.0004320f;
      s.sepS = 0.0015f;
      s.scottie = true;
      break;
    case SstvMode::ScottieS2:
      s.vis = 56;
      s.height = 256;
      s.syncS = 0.009f;
      s.porchS = 0.0015f;
      s.pixelS = 0.0002160f;
      s.sepS = 0.0015f;
      s.scottie = true;
      break;
    case SstvMode::Robot36:
      s.vis = 8;
      s.height = 240;
      s.syncS = 0.009f;
      s.porchS = 0.003f;
      s.pixelS = 0.0002750f;
      s.sepS = 0.0000f;
      s.channels = 2;
      s.robot = true;
      break;
    default:
      break;
  }
  return s;
}

}
