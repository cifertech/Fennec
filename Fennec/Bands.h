#pragma once

#include <Arduino.h>
#include "Config.h"


namespace fennec {

enum class BandKind : uint8_t { FM, AM, SW };

struct Band {
  const char* name;
  const char* title;
  BandKind kind;
  uint16_t minFreq;
  uint16_t maxFreq;
  uint16_t defaultFreq;
  uint16_t step;
  uint8_t amFilter;
};

constexpr uint8_t kAmFilterMw = 2;
constexpr uint8_t kAmFilterSw = 1;

constexpr Band kBands[] = {
    {"FULL", "FM 64-108", BandKind::FM, 6400, 10800, 10000, 5, 0},
    {"OIRT", "OIRT FM", BandKind::FM, 6580, 7400, 7000, 3, 0},
    {"JP", "Japan FM", BandKind::FM, 7600, 9500, 8250, 10, 0},
    {"ITU", "ITU FM", BandKind::FM, 8750, 10800, 10000, 10, 0},

    {"FULL", "AM 153-1710", BandKind::AM, 153, 1710, 693, 1, kAmFilterMw},
    {"LW", "Long wave", BandKind::AM, 153, 279, 198, 1, kAmFilterMw},
    {"MW", "Medium wave", BandKind::AM, 520, 1710, 693, FENNEC_MW_STEP_KHZ, kAmFilterMw},

    {"FULL", "SW 1.71-27.9", BandKind::SW, 1710, 27900, 6075, 5, kAmFilterSw},
    {"160m", "160 m amateur", BandKind::SW, 1800, 2000, 1840, 1, kAmFilterSw},
    {"120m", "120 m tropic", BandKind::SW, 2300, 2495, 2400, 5, kAmFilterSw},
    {"90m", "90 m tropic", BandKind::SW, 3200, 3400, 3300, 5, kAmFilterSw},
    {"75m", "75 m", BandKind::SW, 3900, 4000, 3955, 5, kAmFilterSw},
    {"60m", "60 m", BandKind::SW, 4750, 5060, 4885, 5, kAmFilterSw},
    {"49m", "49 m", BandKind::SW, 5900, 6200, 6075, 5, kAmFilterSw},
    {"41m", "41 m", BandKind::SW, 7200, 7600, 7325, 5, kAmFilterSw},
    {"31m", "31 m", BandKind::SW, 9400, 9900, 9510, 5, kAmFilterSw},
    {"25m", "25 m", BandKind::SW, 11600, 12100, 11800, 5, kAmFilterSw},
    {"22m", "22 m", BandKind::SW, 13570, 13870, 13630, 5, kAmFilterSw},
    {"19m", "19 m", BandKind::SW, 15100, 15800, 15300, 5, kAmFilterSw},
    {"16m", "16 m", BandKind::SW, 17480, 17900, 17650, 5, kAmFilterSw},
    {"15m", "15 m", BandKind::SW, 18900, 19020, 18980, 5, kAmFilterSw},
    {"13m", "13 m", BandKind::SW, 21450, 21850, 21500, 5, kAmFilterSw},
    {"11m", "11 m", BandKind::SW, 25600, 26100, 25800, 5, kAmFilterSw},
    {"CB", "CB 11 m", BandKind::SW, 26965, 27405, 27185, 5, kAmFilterSw},
};

constexpr uint8_t kBandCount = sizeof(kBands) / sizeof(kBands[0]);

inline const Band& bandAt(uint8_t i) {
  if (i >= kBandCount) {
    i = 0;
  }
  return kBands[i];
}

inline uint8_t firstIndexOf(BandKind kind) {
  for (uint8_t i = 0; i < kBandCount; ++i) {
    if (kBands[i].kind == kind) {
      return i;
    }
  }
  return 0;
}

inline uint8_t slotCountOf(BandKind kind) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < kBandCount; ++i) {
    if (kBands[i].kind == kind) {
      ++n;
    }
  }
  return n;
}

inline uint8_t indexFromSlot(BandKind kind, uint8_t slot) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < kBandCount; ++i) {
    if (kBands[i].kind == kind) {
      if (n == slot) {
        return i;
      }
      ++n;
    }
  }
  return firstIndexOf(kind);
}

inline uint32_t bandSpan(uint8_t i) {
  const Band& b = bandAt(i);
  return static_cast<uint32_t>(b.maxFreq) - b.minFreq;
}

inline bool bandContains(uint8_t i, uint16_t freq) {
  if (i >= kBandCount) {
    return false;
  }
  return freq >= kBands[i].minFreq && freq <= kBands[i].maxFreq;
}

inline int16_t tightestContaining(BandKind kind, uint16_t freq) {
  int16_t best = -1;
  uint32_t bestSpan = 0xFFFFFFFFu;
  for (uint8_t i = 0; i < kBandCount; ++i) {
    if (kBands[i].kind != kind || !bandContains(i, freq)) {
      continue;
    }
    const uint32_t s = bandSpan(i);
    if (s < bestSpan) {
      bestSpan = s;
      best = static_cast<int16_t>(i);
    }
  }
  return best;
}

inline bool resolveTune(float f, uint8_t current, uint8_t* bandOut, uint16_t* freqOut) {
  if (f <= 0.0f || bandOut == nullptr || freqOut == nullptr) {
    return false;
  }
  if (current >= kBandCount) {
    current = 0;
  }

  if (f >= 64.0f && f <= 108.0f) {
    const uint16_t u = static_cast<uint16_t>(f * 100.0f + 0.5f);
    int16_t i = tightestContaining(BandKind::FM, u);
    if (i < 0) {
      return false;
    }
    if (bandAt(current).kind == BandKind::FM && bandContains(current, u)) {
      i = static_cast<int16_t>(current);
    }
    *bandOut = static_cast<uint8_t>(i);
    *freqOut = u;
    return true;
  }

  if (f >= 153.0f && f <= 27900.0f) {
    const uint16_t u = static_cast<uint16_t>(f + 0.5f);
    const int16_t am = tightestContaining(BandKind::AM, u);
    const int16_t sw = tightestContaining(BandKind::SW, u);
    int16_t i = -1;
    if (am >= 0 && sw >= 0) {
      i = bandSpan(static_cast<uint8_t>(am)) <= bandSpan(static_cast<uint8_t>(sw)) ? am : sw;
    } else if (am >= 0) {
      i = am;
    } else {
      i = sw;
    }
    if (i < 0) {
      return false;
    }
    if (bandAt(current).kind != BandKind::FM && bandContains(current, u)) {
      i = static_cast<int16_t>(current);
    }
    *bandOut = static_cast<uint8_t>(i);
    *freqOut = u;
    return true;
  }

  if (f >= 6400.0f && f <= 10800.0f) {
    const uint16_t u = static_cast<uint16_t>(f + 0.5f);
    int16_t i = tightestContaining(BandKind::FM, u);
    if (i < 0) {
      return false;
    }
    if (bandAt(current).kind == BandKind::FM && bandContains(current, u)) {
      i = static_cast<int16_t>(current);
    }
    *bandOut = static_cast<uint8_t>(i);
    *freqOut = u;
    return true;
  }

  return false;
}

}
