#pragma once

#include <Arduino.h>
#include "Bands.h"

namespace fennec {

struct SignalQuality {
  uint8_t rssi = 0;
  uint8_t snr = 0;
  bool stereo = false;
};

class Radio {
 public:
  bool begin();
  bool isPresent() const { return present_; }

  void setBand(uint8_t index);
  void nextBand();
  void prevBand();
  uint8_t bandIndex() const { return bandIndex_; }
  const Band& band() const { return bandAt(bandIndex_); }
  bool isFM() const { return !ssbOn_ && band().kind == BandKind::FM; }
  bool ssbOn() const { return ssbOn_; }
  bool ssbUsb() const { return ssbUsb_; }
  int16_t bfo() const { return bfo_; }

  bool enterSsb(uint16_t freqKhz, bool usb);
  void setSsbUsb(bool usb);
  void setBfo(int16_t hz);
  void nudgeBfo(int16_t delta);
  void setSsbAudioBw(uint8_t idx);

  void tuneUp(uint8_t steps = 1);
  void tuneDown(uint8_t steps = 1);
  void setFrequency(uint16_t freq);
  uint16_t frequency() const { return frequency_; }

  void seekUp();
  void seekDown();

  void setVolume(uint8_t vol);
  uint8_t volume() const { return volume_; }

  void setSoftMute(bool on);
  bool softMuted() const { return softMuted_; }

  SignalQuality readSignal();
  void pollRds();
  const char* rdsName() const { return rdsName_; }

 private:
  void applyBand(const Band& b, uint16_t freq);
  uint16_t clampFreq(const Band& b, uint16_t freq) const;
  void remember();

  bool present_ = false;
  bool ssbOn_ = false;
  bool ssbUsb_ = true;
  int16_t bfo_ = 0;
  uint8_t bandIndex_ = 0;
  uint16_t frequency_ = 10000;
  uint8_t volume_ = FENNEC_DEFAULT_VOLUME;
  bool softMuted_ = false;
  uint16_t lastFreq_[kBandCount] = {};
  char rdsName_[9] = {0};
};

}
