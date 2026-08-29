#include "Radio.h"

#include <Wire.h>
#include <SI4735.h>
#include <patch_ssb_compressed.h>

#include "Config.h"
#include "Pins.h"

namespace fennec {

namespace {
SI4735 gChip;

uint16_t fmSeekSpacing(uint16_t step) {
  if (step >= 20) {
    return 20;
  }
  if (step >= 10) {
    return 10;
  }
  return 5;
}

uint16_t amSeekSpacing(uint16_t step) {
  if (step >= 10) {
    return 10;
  }
  if (step == 9) {
    return 9;
  }
  if (step >= 5) {
    return 5;
  }
  return 1;
}
}

bool Radio::begin() {
  for (uint8_t i = 0; i < kBandCount; ++i) {
    lastFreq_[i] = kBands[i].defaultFreq;
  }

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(FENNEC_I2C_HZ);

#if FENNEC_SIMULATE_RADIO
  present_ = true;
  bandIndex_ = 0;
  frequency_ = kBands[0].defaultFreq;
  Serial.println(F("[radio] simulation mode - no SI4732 I2C"));
  return true;
#endif

  const int16_t addr = gChip.getDeviceI2CAddress(PIN_RADIO_RESET);
  if (addr == 0) {
    Serial.println(F("[radio] SI4732 not found on I2C"));
    present_ = false;
    frequency_ = kBands[0].defaultFreq;
    return false;
  }

  Serial.printf("[radio] SI4732 at 0x%02X\n", addr);
  gChip.setup(PIN_RADIO_RESET, POWER_UP_FM);
  present_ = true;
  applyBand(kBands[0], kBands[0].defaultFreq);
  setVolume(volume_);
  return true;
}

void Radio::setBand(uint8_t index) {
  if (index >= kBandCount) {
    index = 0;
  }
  remember();
  bandIndex_ = index;
  applyBand(kBands[index], lastFreq_[index]);
  rdsName_[0] = 0;
}

void Radio::nextBand() {
  setBand(static_cast<uint8_t>((bandIndex_ + 1) % kBandCount));
}

void Radio::prevBand() {
  setBand(static_cast<uint8_t>((bandIndex_ + kBandCount - 1) % kBandCount));
}

void Radio::tuneUp(uint8_t steps) {
  if (ssbOn_) {
    const uint32_t next = static_cast<uint32_t>(frequency_) + steps;
    setFrequency(next > 27900 ? 1710 : static_cast<uint16_t>(next));
    return;
  }
  const Band& b = band();
  const uint32_t next = static_cast<uint32_t>(frequency_) + static_cast<uint32_t>(b.step) * steps;
  setFrequency(next > b.maxFreq ? b.minFreq : static_cast<uint16_t>(next));
}

void Radio::tuneDown(uint8_t steps) {
  if (ssbOn_) {
    const int32_t next = static_cast<int32_t>(frequency_) - steps;
    setFrequency(next < 1710 ? 27900 : static_cast<uint16_t>(next));
    return;
  }
  const Band& b = band();
  const int32_t next = static_cast<int32_t>(frequency_) - static_cast<int32_t>(b.step) * steps;
  setFrequency(next < b.minFreq ? b.maxFreq : static_cast<uint16_t>(next));
}

void Radio::setFrequency(uint16_t freq) {
  const Band& b = band();
  frequency_ = clampFreq(b, freq);
  lastFreq_[bandIndex_] = frequency_;
  if (!present_) {
    return;
  }
#if !FENNEC_SIMULATE_RADIO
  gChip.setFrequency(frequency_);
  frequency_ = gChip.getFrequency();
  lastFreq_[bandIndex_] = frequency_;
#endif
  rdsName_[0] = 0;
}

void Radio::seekUp() {
  if (!present_) {
    tuneUp(10);
    return;
  }
#if FENNEC_SIMULATE_RADIO
  tuneUp(10);
#else
  gChip.seekStationUp();
  frequency_ = gChip.getFrequency();
  lastFreq_[bandIndex_] = frequency_;
#endif
  rdsName_[0] = 0;
}

void Radio::seekDown() {
  if (!present_) {
    tuneDown(10);
    return;
  }
#if FENNEC_SIMULATE_RADIO
  tuneDown(10);
#else
  gChip.seekStationDown();
  frequency_ = gChip.getFrequency();
  lastFreq_[bandIndex_] = frequency_;
#endif
  rdsName_[0] = 0;
}

void Radio::setVolume(uint8_t vol) {
  if (vol > 63) {
    vol = 63;
  }
  volume_ = vol;
  if (!present_ || softMuted_) {
    return;
  }
#if !FENNEC_SIMULATE_RADIO
  gChip.setVolume(volume_);
#endif
}

void Radio::setSoftMute(bool on) {
  softMuted_ = on;
  if (!present_) {
    return;
  }
#if !FENNEC_SIMULATE_RADIO
  gChip.setAudioMute(on);
  if (!on) {
    gChip.setVolume(volume_);
  }
#endif
}

SignalQuality Radio::readSignal() {
  SignalQuality q;
  if (!present_) {
    return q;
  }
#if FENNEC_SIMULATE_RADIO
  q.rssi = 28;
  q.snr = 12;
  q.stereo = isFM();
  return q;
#else
  gChip.getCurrentReceivedSignalQuality();
  if (isFM()) {
    q.rssi = gChip.getCurrentRSSI();
    q.snr = gChip.getCurrentSNR();
    q.stereo = gChip.getCurrentPilot();
  } else {
    const uint8_t resp3 = static_cast<uint8_t>(
        gChip.getCurrentStereoBlend() | (gChip.getCurrentPilot() ? 0x80 : 0));
    const uint8_t resp4 = gChip.getCurrentRSSI();
    const uint8_t resp5 = gChip.getCurrentSNR();
    q.rssi = resp3;
    q.snr = resp4;
    if (q.rssi < 2 && resp4 >= 2) {
      q.rssi = resp4;
      q.snr = resp5;
    }
    q.stereo = false;
  }
  return q;
#endif
}

void Radio::pollRds() {
  rdsName_[8] = 0;
  if (!present_ || !isFM()) {
    rdsName_[0] = 0;
    return;
  }
#if FENNEC_SIMULATE_RADIO
  return;
#else
  gChip.getRdsStatus();
  if (!gChip.getRdsReceived()) {
    return;
  }
  char* name = gChip.getRdsText0A();
  if (name == nullptr) {
    return;
  }
  strncpy(rdsName_, name, 8);
  rdsName_[8] = 0;
#endif
}

void Radio::applyBand(const Band& b, uint16_t freq) {
  if (!present_) {
    ssbOn_ = false;
    frequency_ = clampFreq(b, freq);
    lastFreq_[bandIndex_] = frequency_;
    return;
  }
#if FENNEC_SIMULATE_RADIO
  ssbOn_ = false;
  frequency_ = clampFreq(b, freq);
  lastFreq_[bandIndex_] = frequency_;
  return;
#else
  if (ssbOn_) {
    gChip.setup(PIN_RADIO_RESET, b.kind == BandKind::FM ? POWER_UP_FM : POWER_UP_AM);
    ssbOn_ = false;
    bfo_ = 0;
  }
  frequency_ = clampFreq(b, freq);
  lastFreq_[bandIndex_] = frequency_;
  if (b.kind == BandKind::FM) {
    gChip.setFM(b.minFreq, b.maxFreq, frequency_, b.step);
    gChip.setSeekFmLimits(b.minFreq, b.maxFreq);
    gChip.setSeekFmSpacing(fmSeekSpacing(b.step));
    gChip.setTuneFrequencyAntennaCapacitor(0);
    gChip.setRdsConfig(1, 0, 0, 0, 0);
  } else {
    gChip.setAM(b.minFreq, b.maxFreq, frequency_, b.step);
    gChip.setSeekAmLimits(b.minFreq, b.maxFreq);
    gChip.setSeekAmSpacing(amSeekSpacing(b.step));
    gChip.setTuneFrequencyAntennaCapacitor(b.kind == BandKind::SW ? 1 : 0);
    gChip.setBandwidth(b.amFilter, 1);
    gChip.setAvcAmMaxGain(48);
  }
  gChip.setVolume(softMuted_ ? 0 : volume_);
  frequency_ = gChip.getFrequency();
  lastFreq_[bandIndex_] = frequency_;
#endif
}

uint16_t Radio::clampFreq(const Band& b, uint16_t freq) const {
  if (ssbOn_) {
    if (freq < 1710) {
      return 1710;
    }
    if (freq > 27900) {
      return 27900;
    }
    return freq;
  }
  if (freq < b.minFreq) {
    return b.minFreq;
  }
  if (freq > b.maxFreq) {
    return b.maxFreq;
  }
  return freq;
}

void Radio::remember() {
  lastFreq_[bandIndex_] = frequency_;
}

bool Radio::enterSsb(uint16_t freqKhz, bool usb) {
  if (freqKhz < 1710) {
    freqKhz = 1710;
  }
  if (freqKhz > 27900) {
    freqKhz = 27900;
  }
  ssbUsb_ = usb;
  bfo_ = 0;
  frequency_ = freqKhz;
#if FENNEC_SIMULATE_RADIO
  ssbOn_ = true;
  present_ = true;
  Serial.printf("[radio] SSB sim %s %u kHz\n", usb ? "USB" : "LSB", freqKhz);
  return true;
#endif
  if (!present_) {
    return false;
  }
  gChip.setup(PIN_RADIO_RESET, POWER_UP_AM);
  delay(15);
  gChip.setI2CFastModeCustom(500000);
  gChip.queryLibraryId();
  gChip.patchPowerUp();
  delay(50);
  gChip.downloadCompressedPatch(ssb_patch_content, sizeof(ssb_patch_content), cmd_0x15,
                                sizeof(cmd_0x15));
  gChip.setI2CStandardMode();
  Wire.setClock(FENNEC_I2C_HZ);
  gChip.setTuneFrequencyAntennaCapacitor(1);
  gChip.setSSB(1710, 27900, freqKhz, 1, usb ? USB_MODE : LSB_MODE);
  gChip.setSSBConfig(3, 1, 0, 1, 0, 1);
  gChip.setSSBBfo(0);
  gChip.setAvcAmMaxGain(52);
  gChip.setVolume(softMuted_ ? 0 : volume_);
  ssbOn_ = true;
  frequency_ = gChip.getFrequency();
  Serial.printf("[radio] SSB %s %u kHz (patch loaded)\n", usb ? "USB" : "LSB", frequency_);
  return true;
}

void Radio::setSsbUsb(bool usb) {
  ssbUsb_ = usb;
  if (!ssbOn_ || !present_) {
    return;
  }
#if FENNEC_SIMULATE_RADIO
  return;
#else
  gChip.setSSB(1710, 27900, frequency_, 1, usb ? USB_MODE : LSB_MODE);
  gChip.setSSBBfo(bfo_);
  gChip.setVolume(softMuted_ ? 0 : volume_);
  frequency_ = gChip.getFrequency();
#endif
}

void Radio::setBfo(int16_t hz) {
  if (hz > 16000) {
    hz = 16000;
  }
  if (hz < -16000) {
    hz = -16000;
  }
  bfo_ = hz;
  if (!present_ || !ssbOn_) {
    return;
  }
#if !FENNEC_SIMULATE_RADIO
  gChip.setSSBBfo(bfo_);
#endif
}

void Radio::nudgeBfo(int16_t delta) {
  setBfo(static_cast<int16_t>(bfo_ + delta));
}

void Radio::setSsbAudioBw(uint8_t idx) {
  if (idx > 5) {
    idx = 5;
  }
  if (!present_ || !ssbOn_) {
    return;
  }
#if !FENNEC_SIMULATE_RADIO
  gChip.setSSBConfig(idx, 1, 0, 1, 0, 1);
#endif
}

}
