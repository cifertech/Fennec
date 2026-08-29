#include "NsLogger.h"

#include <Preferences.h>
#include <string.h>

namespace fennec {

namespace {
const NsPreset kPresets[kNsPresetCount] = {
    {"4625", 4625, false, true},
    {"5448", 5448, true, false},
    {"6925", 6925, true, false},
    {"9074", 9074, true, false},
    {"HERE", 0, true, false},
};

Preferences nsPrefs;
}

const NsPreset* NsLogger::presets() { return kPresets; }

void NsLogger::begin() { load(); }

void NsLogger::start() {
  active_ = true;
  open_ = false;
  aboveSince_ = belowSince_ = 0;
  lastMs_ = 0;
  Serial.println(F("[watch] numbers net - ARM to record RSSI events"));
}

void NsLogger::stop() {
  if (open_) {
    open_ = false;
    save();
  }
  active_ = false;
  armed_ = false;
}

void NsLogger::setArmed(bool on) {
  armed_ = on;
  open_ = false;
  aboveSince_ = belowSince_ = 0;
}

void NsLogger::clear() {
  count_ = 0;
  head_ = 0;
  save();
}

void NsLogger::setThreshold(uint8_t t) {
  if (t < 5) {
    t = 5;
  }
  if (t > 80) {
    t = 80;
  }
  thr_ = t;
}

void NsLogger::nudgeThreshold(int8_t d) {
  setThreshold(static_cast<uint8_t>(static_cast<int>(thr_) + d));
}

void NsLogger::setPreset(uint8_t i) {
  if (i >= kNsPresetCount) {
    i = 0;
  }
  preset_ = i;
}

void NsLogger::applyPreset(Radio& radio) {
  const NsPreset& p = kPresets[preset_];
  uint16_t f = p.khz;
  if (f == 0) {
    f = radio.frequency();
    if (radio.isFM()) {
      return;
    }
  }
  if (p.am) {
    if (radio.ssbOn()) {
    }
    radio.setFrequency(f);
  } else {
    if (!radio.ssbOn()) {
      radio.enterSsb(f, p.usb);
    } else {
      radio.setSsbUsb(p.usb);
      radio.setFrequency(f);
    }
  }
}

const NsEvent& NsLogger::at(uint8_t i) const {
  static NsEvent empty{};
  if (i >= count_) {
    return empty;
  }
  const int idx = static_cast<int>(head_) - 1 - static_cast<int>(i);
  const uint8_t u = static_cast<uint8_t>((idx + kNsLogCap) % kNsLogCap);
  return log_[u];
}

void NsLogger::poll(Radio& radio, const char* nowIso, const char* cwTail) {
  if (!active_ || !armed_) {
    return;
  }
  const uint32_t now = millis();
  if (now - lastMs_ < 80) {
    return;
  }
  lastMs_ = now;
  const SignalQuality q = radio.readSignal();
  const bool hot = q.rssi >= thr_;
  if (hot) {
    belowSince_ = 0;
    if (aboveSince_ == 0) {
      aboveSince_ = now;
    }
    if (q.rssi > peakRssi_) {
      peakRssi_ = q.rssi;
    }
    if (q.snr > peakSnr_) {
      peakSnr_ = q.snr;
    }
    if (!open_ && (now - aboveSince_) >= 2000) {
      open_ = true;
      memset(&cur_, 0, sizeof(cur_));
      strncpy(cur_.when, nowIso ? nowIso : "", sizeof(cur_.when) - 1);
      cur_.freq = radio.frequency();
      cur_.usb = radio.ssbUsb();
      cur_.am = !radio.ssbOn();
      Serial.printf("[log] open  %u kHz  rssi %u\n", cur_.freq, q.rssi);
    }
    if (open_ && cwTail && cwTail[0]) {
      strncpy(cur_.cw, cwTail, sizeof(cur_.cw) - 1);
    }
  } else {
    aboveSince_ = 0;
    if (belowSince_ == 0) {
      belowSince_ = now;
    }
    if (open_ && (now - belowSince_) >= 5000) {
      cur_.peakRssi = peakRssi_;
      cur_.peakSnr = peakSnr_;
      pushEvent(cur_);
      save();
      Serial.printf("[log] close %u kHz  peak %u\n", cur_.freq, peakRssi_);
      open_ = false;
      peakRssi_ = peakSnr_ = 0;
    }
  }
}

void NsLogger::pushEvent(const NsEvent& e) {
  log_[head_] = e;
  head_ = static_cast<uint8_t>((head_ + 1) % kNsLogCap);
  if (count_ < kNsLogCap) {
    ++count_;
  }
}

void NsLogger::load() {
  if (!nsPrefs.begin("fennec", true)) {
    return;
  }
  count_ = nsPrefs.getUChar("nsN", 0);
  head_ = nsPrefs.getUChar("nsH", 0);
  thr_ = nsPrefs.getUChar("nsThr", 25);
  if (count_ > kNsLogCap) {
    count_ = 0;
  }
  nsPrefs.getBytes("nsLog", log_, sizeof(log_));
  nsPrefs.end();
}

void NsLogger::save() {
  if (!nsPrefs.begin("fennec", false)) {
    return;
  }
  nsPrefs.putUChar("nsN", count_);
  nsPrefs.putUChar("nsH", head_);
  nsPrefs.putUChar("nsThr", thr_);
  nsPrefs.putBytes("nsLog", log_, sizeof(log_));
  nsPrefs.end();
}

void NsLogger::dumpJson(char* out, size_t n) const {
  if (out == nullptr || n < 16) {
    return;
  }
  int p = snprintf(out, n, "{\"ok\":true,\"armed\":%s,\"count\":%u,\"events\":[",
                   armed_ ? "true" : "false", count_);
  for (uint8_t i = 0; i < count_; ++i) {
    if (p < 0 || static_cast<size_t>(p) + 90 >= n) {
      break;
    }
    const NsEvent& e = at(i);
    p += snprintf(out + p, n - static_cast<size_t>(p),
                  "%s{\"when\":\"%s\",\"freq\":%u,\"rssi\":%u,\"snr\":%u,\"cw\":\"%s\"}",
                  i ? "," : "", e.when, e.freq, e.peakRssi, e.peakSnr, e.cw);
  }
  if (p > 0 && static_cast<size_t>(p) + 3 < n) {
    snprintf(out + p, n - static_cast<size_t>(p), "]}");
  }
}

void NsLogger::dumpSerial() const {
  Serial.printf("[log] %u events (thr %u)\n", count_, thr_);
  for (uint8_t i = 0; i < count_; ++i) {
    const NsEvent& e = at(i);
    Serial.printf("  %s  %u kHz  rssi %u  snr %u  %s\n", e.when, e.freq, e.peakRssi, e.peakSnr,
                  e.cw);
  }
}

}
