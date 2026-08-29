#include "CwDecoder.h"

#include <math.h>
#include <string.h>

#include "AudioTap.h"
#include "Config.h"

namespace fennec {

namespace {
constexpr float kPi = 3.14159265f;
constexpr uint16_t kBlk = 32;
constexpr float kDtBlk = static_cast<float>(kBlk) / static_cast<float>(AdcTap::kFs);

char morseLookup(uint16_t bits, uint8_t n) {
  struct Entry {
    uint16_t bits;
    uint8_t n;
    char c;
  };
  static const Entry kTab[] = {
      {0b01, 2, 'A'},    {0b1000, 4, 'B'}, {0b1010, 4, 'C'}, {0b100, 3, 'D'},   {0b0, 1, 'E'},
      {0b0010, 4, 'F'},  {0b110, 3, 'G'},  {0b0000, 4, 'H'}, {0b00, 2, 'I'},    {0b0111, 4, 'J'},
      {0b101, 3, 'K'},   {0b0100, 4, 'L'}, {0b11, 2, 'M'},   {0b10, 2, 'N'},    {0b111, 3, 'O'},
      {0b0110, 4, 'P'},  {0b1101, 4, 'Q'}, {0b010, 3, 'R'},  {0b000, 3, 'S'},   {0b1, 1, 'T'},
      {0b001, 3, 'U'},   {0b0001, 4, 'V'}, {0b011, 3, 'W'},  {0b1001, 4, 'X'},  {0b1011, 4, 'Y'},
      {0b1100, 4, 'Z'},  {0b01111, 5, '1'}, {0b00111, 5, '2'}, {0b00011, 5, '3'}, {0b00001, 5, '4'},
      {0b00000, 5, '5'}, {0b10000, 5, '6'}, {0b11000, 5, '7'}, {0b11100, 5, '8'}, {0b11110, 5, '9'},
      {0b11111, 5, '0'}, {0b010101, 6, '.'}, {0b110011, 6, ','}, {0b001100, 6, '?'},
      {0b10001, 5, '/'}, {0b100001, 6, '-'}, {0b01010, 5, '+'}, {0b111000, 6, ':'},
  };
  for (uint8_t i = 0; i < sizeof(kTab) / sizeof(kTab[0]); ++i) {
    if (kTab[i].n == n && kTab[i].bits == bits) {
      return kTab[i].c;
    }
  }
  return 0;
}
}

void CwDecoder::begin() {
  AdcTap::instance().begin();
  MicTap::instance().begin();
}

void CwDecoder::setSource(CwSource src) {
  if (source_ == src) {
    return;
  }
  const bool live = running_;
  source_ = src;
  if (live) {
    start();
  }
}

void CwDecoder::setPitch(uint16_t hz) {
  if (hz < 400) {
    hz = 400;
  }
  if (hz > 1200) {
    hz = 1200;
  }
  pitch_ = hz;
  goCoef_ = 2.0f * cosf(2.0f * kPi * static_cast<float>(pitch_) / static_cast<float>(AdcTap::kFs));
}

void CwDecoder::nudgePitch(int16_t delta) {
  setPitch(static_cast<uint16_t>(static_cast<int16_t>(pitch_) + delta));
}

void CwDecoder::start() {
  begin();
  stop();
  setPitch(pitch_);
  env_ = 0;
  noise_ = 1.0f;
  onS_ = offS_ = 0;
  unitS_ = 0.060f;
  idle_ = true;
  trained_ = false;
  pendingN_ = 0;
  peakTone_ = 0;
  bits_ = 0;
  nBits_ = 0;
  keyed_ = false;
  q0_ = 0;
  q1_ = 0;
  blk_ = 0;
  textLen_ = 0;
  text_[0] = 0;
  wpm_ = 18;
  hasAudio_ = false;
  peakN_ = peakAcc_ = 0;
  running_ = true;
  const bool ok = source_ == CwSource::Mic ? MicTap::instance().start() : AdcTap::instance().start();
  if (!ok) {
    running_ = false;
    return;
  }
  Serial.printf("[cw] %s  pitch %u Hz\n", source_ == CwSource::Mic ? "MIC" : "RADIO", pitch_);
}

void CwDecoder::stop() {
  running_ = false;
  AdcTap::instance().stop();
  MicTap::instance().stop();
}

bool CwDecoder::popSample(int16_t* s) {
  if (source_ == CwSource::Mic) {
    return MicTap::instance().pop(s);
  }
  return AdcTap::instance().pop(s);
}

void CwDecoder::poll() {
  if (!running_) {
    return;
  }
  int16_t s = 0;
  uint16_t n = 0;
  const uint32_t t0 = millis();
  float q0 = q0_;
  float q1 = q1_;
  const float coeff = goCoef_;
  if (source_ == CwSource::Mic) {
    MicTap::instance().poll();
  }
  while (n < 800 && popSample(&s)) {
    const float x = static_cast<float>(s);
    const float q2 = coeff * q1 - q0 + x;
    q0 = q1;
    q1 = q2;
    ++blk_;
    const uint16_t a = static_cast<uint16_t>(s < 0 ? -s : s);
    peakAcc_ += a;
    ++peakN_;
    if (blk_ >= kBlk) {
      const float mag = q1 * q1 + q0 * q0 - coeff * q1 * q0;
      pushMag(mag < 0 ? 0 : mag, kDtBlk);
      q0 = q1 = 0;
      blk_ = 0;
    }
    ++n;
    if ((n & 127) == 0 && (millis() - t0) >= 18) {
      break;
    }
  }
  q0_ = q0;
  q1_ = q1;
  yield();
  if (peakN_ >= 400) {
    hasAudio_ = (peakAcc_ / peakN_) > 40;
    peakAcc_ = 0;
    peakN_ = 0;
  }
}

void CwDecoder::pushMag(float mag, float dt) {
  if (mag > env_) {
    env_ = env_ * 0.30f + mag * 0.70f;
  } else {
    env_ = env_ * 0.15f + mag * 0.85f;
  }
  if (env_ < noise_) {
    noise_ = noise_ * 0.65f + env_ * 0.35f;
  } else if (!keyed_) {
    noise_ = noise_ * 0.997f + env_ * 0.003f;
  }
  if (noise_ < 1.0f) {
    noise_ = 1.0f;
  }

  const float absMin = source_ == CwSource::Mic ? 6.0e4f : 3.0e5f;
  const float gateOn = noise_ * 5.5f + absMin;
  const float gateOff = gateOn * 0.42f;
  const float floor = peakTone_ > absMin ? peakTone_ * 0.10f : 0.0f;
  bool on = keyed_ ? (env_ > gateOff) : (env_ > gateOn);
  if (floor > 0.0f && env_ < floor) {
    on = false;
  }

  if (!on) {
    offS_ += dt;
    peakTone_ *= 0.990f;
    if (!trained_ && pendingN_ > 0) {
      pendingGap_[pendingN_ - 1] = offS_;
    }
    if (offS_ > 0.90f) {
      if (!trained_ && pendingN_ > 0) {
        trainFromPending();
      } else {
        flushChar();
      }
      idle_ = true;
      pendingN_ = 0;
      keyed_ = false;
      onS_ = 0;
      bits_ = 0;
      nBits_ = 0;
      return;
    }
    const float closeS = trained_ ? unitS_ * 0.38f : 0.016f;
    const float minOn = trained_ ? unitS_ * 0.22f : 0.010f;
    if (keyed_ && offS_ > closeS) {
      if (!idle_ && onS_ >= minOn) {
        if (!trained_) {
          noteMark(onS_);
          if (pendingN_ >= 8) {
            trainFromPending();
          }
        } else {
          const bool dash = onS_ > unitS_ * 2.0f;
          if (nBits_ < 8) {
            bits_ = static_cast<uint16_t>((bits_ << 1) | (dash ? 1 : 0));
            ++nBits_;
          }
          const float u = dash ? onS_ / 3.0f : onS_;
          if (u >= 0.016f && u <= 0.160f) {
            setUnit(unitS_ * 0.70f + u * 0.30f);
          }
        }
      }
      keyed_ = false;
      onS_ = 0;
    }
    if (trained_ && offS_ > unitS_ * 2.0f && nBits_ > 0) {
      flushChar();
    }
    if (!trained_ && pendingN_ >= 4 && offS_ > 0.32f) {
      trainFromPending();
    }
    return;
  }

  if (env_ > peakTone_) {
    peakTone_ = env_;
  }
  onS_ += dt;
  if (idle_) {
    if (onS_ < 0.010f) {
      return;
    }
    idle_ = false;
    keyed_ = true;
    offS_ = 0;
    return;
  }
  if (!keyed_) {
    if (offS_ > unitS_ * 5.0f) {
      flushChar();
      addChar(' ');
    } else if (offS_ > unitS_ * 1.8f) {
      flushChar();
    }
    keyed_ = true;
    onS_ = dt;
  }
  offS_ = 0;
}

void CwDecoder::flushChar() {
  if (nBits_ == 0) {
    return;
  }
  const char c = lookup(bits_, nBits_);
  bits_ = 0;
  nBits_ = 0;
  if (c != 0) {
    addChar(c);
  }
}

void CwDecoder::addChar(char c) {
  if (c == ' ' && (textLen_ == 0 || text_[textLen_ - 1] == ' ')) {
    return;
  }
  if (textLen_ >= sizeof(text_) - 2) {
    memmove(text_, text_ + 16, textLen_ - 16);
    textLen_ = static_cast<uint8_t>(textLen_ - 16);
  }
  text_[textLen_++] = c;
  text_[textLen_] = 0;
}

char CwDecoder::lookup(uint16_t bits, uint8_t n) { return morseLookup(bits, n); }

void CwDecoder::setUnit(float u) {
  if (u < 0.018f) {
    u = 0.018f;
  }
  if (u > 0.150f) {
    u = 0.150f;
  }
  unitS_ = u;
  const int w = static_cast<int>(1.2f / unitS_ + 0.5f);
  wpm_ = static_cast<uint8_t>(w < 8 ? 8 : (w > 50 ? 50 : w));
}

void CwDecoder::noteMark(float onS) {
  if (pendingN_ >= kPending) {
    trainFromPending();
    if (trained_) {
      const bool dash = onS > unitS_ * 2.0f;
      if (nBits_ < 8) {
        bits_ = static_cast<uint16_t>((bits_ << 1) | (dash ? 1 : 0));
        ++nBits_;
      }
    }
    return;
  }
  pendingOn_[pendingN_] = onS;
  pendingGap_[pendingN_] = 0;
  ++pendingN_;
}

void CwDecoder::trainFromPending() {
  if (pendingN_ == 0) {
    trained_ = true;
    return;
  }
  float mn = pendingOn_[0];
  float mx = pendingOn_[0];
  for (uint8_t i = 1; i < pendingN_; ++i) {
    if (pendingOn_[i] < mn) {
      mn = pendingOn_[i];
    }
    if (pendingOn_[i] > mx) {
      mx = pendingOn_[i];
    }
  }
  float u = mn;
  if (mx > mn * 2.05f) {
    u = mn;
  } else if (mn > 0.090f) {
    u = mn / 3.0f;
  }
  setUnit(u);
  trained_ = true;

  bits_ = 0;
  nBits_ = 0;
  for (uint8_t i = 0; i < pendingN_; ++i) {
    const bool dash = pendingOn_[i] > unitS_ * 2.0f;
    if (nBits_ < 8) {
      bits_ = static_cast<uint16_t>((bits_ << 1) | (dash ? 1 : 0));
      ++nBits_;
    }
    const float g = pendingGap_[i];
    if (g > unitS_ * 5.0f) {
      flushChar();
      addChar(' ');
    } else if (g > unitS_ * 1.8f) {
      flushChar();
    }
  }
  pendingN_ = 0;
}

}
