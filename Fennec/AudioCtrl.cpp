#include "Periph.h"

#include "Pins.h"

namespace fennec {

void AudioCtrl::begin() {
  pinMode(PIN_AMP_SD, OUTPUT);
  enabled_ = false;
  writePin(false);
}

void AudioCtrl::enableAmp(bool on) {
  enabled_ = on;
  writePin(on);
}

void AudioCtrl::writePin(bool enabled) {
#if PIN_AMP_SD_ACTIVE_HIGH
  digitalWrite(PIN_AMP_SD, enabled ? HIGH : LOW);
#else
  digitalWrite(PIN_AMP_SD, enabled ? LOW : HIGH);
#endif
}

}
