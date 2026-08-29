#include "Periph.h"

#include "Config.h"
#include "Pins.h"

namespace fennec {

namespace {
bool isDown(uint8_t pin) { return digitalRead(pin) == LOW; }
}

void Input::begin() {
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_OK, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

  btns_[0].pin = PIN_BTN_UP;
  btns_[0].shortEvt = InputEvent::TuneUp;
  btns_[0].longEvt = InputEvent::VolUp;
  btns_[0].down = false;
  btns_[0].longFired = false;
  btns_[0].tDown = 0;

  btns_[1].pin = PIN_BTN_OK;
  btns_[1].shortEvt = InputEvent::BandNext;
  btns_[1].longEvt = InputEvent::SeekUp;
  btns_[1].down = false;
  btns_[1].longFired = false;
  btns_[1].tDown = 0;

  btns_[2].pin = PIN_BTN_DOWN;
  btns_[2].shortEvt = InputEvent::TuneDown;
  btns_[2].longEvt = InputEvent::VolDown;
  btns_[2].down = false;
  btns_[2].longFired = false;
  btns_[2].tDown = 0;
}

InputEvent Input::poll() {
  const bool up = isDown(PIN_BTN_UP);
  const bool down = isDown(PIN_BTN_DOWN);
  const uint32_t now = millis();
  constexpr uint32_t kChordMs = 450;
  if (up && down) {
    if (!chordDown_) {
      chordDown_ = true;
      chordAt_ = now;
      btns_[0].longFired = true;
      btns_[2].longFired = true;
    } else if (!chordFired_ && (now - chordAt_ >= kChordMs)) {
      chordFired_ = true;
      return InputEvent::Home;
    }
    return InputEvent::None;
  }
  if (chordDown_ && !up && !down) {
    chordDown_ = false;
    chordFired_ = false;
  }

  for (uint8_t i = 0; i < 3; ++i) {
    const InputEvent e = pollOneButton(i);
    if (e != InputEvent::None) {
      return e;
    }
  }
  return InputEvent::None;
}

InputEvent Input::pollOneButton(uint8_t index) {
  Btn& b = btns_[index];
  const bool down = isDown(b.pin);
  const uint32_t now = millis();

  if (down && !b.down) {
    b.down = true;
    b.longFired = false;
    b.tDown = now;
    return InputEvent::None;
  }

  if (down && b.down && !b.longFired && b.longEvt != InputEvent::None &&
      (now - b.tDown >= FENNEC_LONG_PRESS_MS)) {
    b.longFired = true;
    return b.longEvt;
  }

  if (!down && b.down) {
    b.down = false;
    if (!b.longFired && (now - b.tDown >= FENNEC_BTN_DEBOUNCE_MS)) {
      return b.shortEvt;
    }
  }
  return InputEvent::None;
}

}
