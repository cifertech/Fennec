#include "App.h"

#include <Preferences.h>
#include <string.h>

#include "Config.h"
#include "Pins.h"

namespace fennec {

namespace {
Preferences prefs;
}

void App::begin() {
  pixel_.begin();

  audio_.begin();

  Serial.begin(115200);
  delay(150);
  Serial.println();
  Serial.println(F("=================================="));
  Serial.println(F("Fennec"));
  Serial.println(F("Developed by: CiferTech"));
  Serial.println(F("Version:      " FENNEC_VERSION));
  Serial.println(F("=================================="));
  Serial.println(F("Touch: menu = RADIO / SSTV / MORSE / HOMING / WATCH  (idle until you pick one)"));
  Serial.println(F("Buttons: 6=tune+  5=band  4=tune-   (hold: vol+ / seek / vol-)"));
  Serial.println(F("Hold OK at boot, or serial 'c', to calibrate touch."));
  Serial.println(F("serial t=SSTV  w=morse  h=homing  l=watch  d=dump"));

  ui_.begin();
  ui_.showSplash();
  input_.begin();
  if (digitalRead(PIN_BTN_OK) == LOW) {
    delay(40);
    if (digitalRead(PIN_BTN_OK) == LOW) {
      Serial.println(F("[touch] calibration"));
      ui_.calibrate();
    }
  }
  ui_.enterMenu();
  sstv_.begin();
  cw_.begin();
  logger_.begin();
  clock_.begin();

  if (!radio_.begin()) {
    Serial.println(F("[app] continuing without tuner - UI / WiFi still run"));
  }
  lastKind_[0] = firstIndexOf(BandKind::FM);
  lastKind_[1] = firstIndexOf(BandKind::AM);
  lastKind_[2] = firstIndexOf(BandKind::SW);
  loadPrefs();
  rememberBand();

  web_.begin();
  applySettings();

  savedBand_ = radio_.bandIndex();
  savedFreq_ = radio_.frequency();
  muteRadio();
  unmuteAt_ = 0;
  publish();
}

void App::loop() {
  serialPoll();
  pollUnmute();
  pollSave();
  clock_.poll();

  if (web_.staUp() && !staNtpArmed_) {
    staNtpArmed_ = true;
    clock_.onStaUp();
  }

  InputEvent e = input_.poll();
  if (e != InputEvent::None) {
    handleInput(e);
  }

  TouchEvent te;
  te.type = TouchEvent::None;
  te.bandIndex = 0;
  if (ui_.pollTouch(te)) {
    handleTouch(te);
  }

  RemoteMsg msg;
  if (web_.take(msg)) {
    handleRemote(msg);
  }

  if (!toolRadio() || hunt_.active() || logger_.active()) {
    if (hunt_.active()) {
      hunt_.poll(radio_);
    } else {
      pollSignal();
    }
  }

  pollTools();

  web_.poll();
  publish();
  ui_.poll();
}

void App::handleInput(InputEvent e) {
  if (e == InputEvent::Home) {
    if (ui_.screen() != UiScreen::Menu) {
      goHome();
    }
    return;
  }

  switch (ui_.screen()) {
    case UiScreen::Menu:
      handleMenuButtons(e);
      return;
    case UiScreen::Settings:
      handleSettingsButtons(e);
      return;
    case UiScreen::About:
      goHome();
      return;
    case UiScreen::Sstv:
    case UiScreen::Cw:
    case UiScreen::Log:
      handleFocusButtons(e);
      return;
    default:
      break;
  }

  if (toolRadio()) {
    switch (e) {
      case InputEvent::TuneUp:
        radio_.tuneUp(1);
        lastToolHudMs_ = 0;
        saveSoon();
        break;
      case InputEvent::TuneDown:
        radio_.tuneDown(1);
        lastToolHudMs_ = 0;
        saveSoon();
        break;
      case InputEvent::VolUp:
        radio_.setVolume(static_cast<uint8_t>(min(63, static_cast<int>(radio_.volume()) + 2)));
        lastToolHudMs_ = 0;
        saveSoon();
        break;
      case InputEvent::VolDown:
        radio_.setVolume(static_cast<uint8_t>(max(0, static_cast<int>(radio_.volume()) - 2)));
        lastToolHudMs_ = 0;
        saveSoon();
        break;
      case InputEvent::MuteToggle:
        userMuted_ = !userMuted_;
        unmuteAt_ = 0;
        if ((sstv_.active() && sstv_.mic()) || (cw_.active() && cw_.mic())) {
          applySstvSpeaker();
        } else {
          radio_.setSoftMute(userMuted_);
          audio_.enableAmp(!userMuted_);
        }
        break;
      case InputEvent::SeekUp:
      case InputEvent::SeekDown:
      case InputEvent::BandPrev:
        exitTool();
        break;
      case InputEvent::BandNext:
        if (hunt_.active()) {
          hunt_.resetPeak();
        } else {
          radio_.setSsbUsb(!radio_.ssbUsb());
          lastToolHudMs_ = 0;
        }
        break;
      default:
        break;
    }
    return;
  }

  switch (e) {
    case InputEvent::TuneUp:
      radio_.tuneUp();
      saveSoon();
      break;
    case InputEvent::TuneDown:
      radio_.tuneDown();
      saveSoon();
      break;
    case InputEvent::BandNext:
      muteRadio();
      radio_.nextBand();
      rememberBand();
      scheduleUnmute(FENNEC_BAND_SWITCH_MUTE_MS);
      saveSoon();
      break;
    case InputEvent::BandPrev:
      muteRadio();
      radio_.prevBand();
      rememberBand();
      scheduleUnmute(FENNEC_BAND_SWITCH_MUTE_MS);
      saveSoon();
      break;
    case InputEvent::SeekUp:
      muteRadio();
      radio_.seekUp();
      scheduleUnmute(FENNEC_BAND_SWITCH_MUTE_MS);
      saveSoon();
      break;
    case InputEvent::SeekDown:
      muteRadio();
      radio_.seekDown();
      scheduleUnmute(FENNEC_BAND_SWITCH_MUTE_MS);
      saveSoon();
      break;
    case InputEvent::VolUp:
      radio_.setVolume(static_cast<uint8_t>(min(63, static_cast<int>(radio_.volume()) + 2)));
      saveSoon();
      break;
    case InputEvent::VolDown:
      radio_.setVolume(static_cast<uint8_t>(max(0, static_cast<int>(radio_.volume()) - 2)));
      saveSoon();
      break;
    case InputEvent::MuteToggle:
      userMuted_ = !userMuted_;
      radio_.setSoftMute(userMuted_);
      audio_.enableAmp(!userMuted_);
      unmuteAt_ = 0;
      break;
    default:
      break;
  }

  rememberBand();
}

void App::handleMenuButtons(InputEvent e) {
  switch (e) {
    case InputEvent::TuneUp:
      ui_.nudgeBtnFocus(-1);
      break;
    case InputEvent::TuneDown:
      ui_.nudgeBtnFocus(1);
      break;
    case InputEvent::BandNext: {
      TouchEvent ev = ui_.focusEvent();
      if (ev.type != TouchEvent::None) {
        handleTouch(ev);
      }
      break;
    }
    default:
      break;
  }
}

void App::handleSettingsButtons(InputEvent e) {
  switch (e) {
    case InputEvent::TuneUp:
      ui_.nudgeBtnFocus(-1);
      break;
    case InputEvent::TuneDown:
      ui_.nudgeBtnFocus(1);
      break;
    case InputEvent::BandNext: {
      TouchEvent ev = ui_.focusEvent();
      if (ev.type != TouchEvent::None) {
        handleTouch(ev);
      }
      break;
    }
    case InputEvent::SeekUp:
    case InputEvent::SeekDown:
    case InputEvent::BandPrev:
      goHome();
      break;
    default:
      break;
  }
}

void App::handleFocusButtons(InputEvent e) {
  switch (e) {
    case InputEvent::TuneUp:
      ui_.nudgeBtnFocus(-1);
      break;
    case InputEvent::TuneDown:
      ui_.nudgeBtnFocus(1);
      break;
    case InputEvent::BandNext: {
      TouchEvent ev = ui_.focusEvent();
      if (ev.type != TouchEvent::None) {
        handleTouch(ev);
      }
      break;
    }
    case InputEvent::SeekUp:
    case InputEvent::SeekDown:
    case InputEvent::BandPrev:
      goHome();
      break;
    case InputEvent::VolUp:
      radio_.setVolume(static_cast<uint8_t>(min(63, static_cast<int>(radio_.volume()) + 2)));
      lastToolHudMs_ = 0;
      saveSoon();
      break;
    case InputEvent::VolDown:
      radio_.setVolume(static_cast<uint8_t>(max(0, static_cast<int>(radio_.volume()) - 2)));
      lastToolHudMs_ = 0;
      saveSoon();
      break;
    case InputEvent::MuteToggle:
      userMuted_ = !userMuted_;
      unmuteAt_ = 0;
      if ((sstv_.active() && sstv_.mic()) || (cw_.active() && cw_.mic())) {
        applySstvSpeaker();
      } else {
        radio_.setSoftMute(userMuted_);
        audio_.enableAmp(!userMuted_);
      }
      break;
    default:
      break;
  }
}

void App::selectKind(BandKind kind) {
  if (radio_.band().kind == kind) {
    return;
  }
  const uint8_t slot = static_cast<uint8_t>(kind);
  uint8_t i = lastKind_[slot];
  if (bandAt(i).kind != kind) {
    i = firstIndexOf(kind);
  }
  muteRadio();
  radio_.setBand(i);
  rememberBand();
  scheduleUnmute(FENNEC_BAND_SWITCH_MUTE_MS);
  saveSoon();
}

void App::rememberBand() {
  const uint8_t i = radio_.bandIndex();
  lastKind_[static_cast<uint8_t>(bandAt(i).kind)] = i;
}

void App::gotoBandFreq(uint8_t index, uint16_t freq) {
  if (index >= kBandCount) {
    return;
  }
  if (index != radio_.bandIndex()) {
    muteRadio();
    radio_.setBand(index);
    rememberBand();
    scheduleUnmute(FENNEC_BAND_SWITCH_MUTE_MS);
  }
  radio_.setFrequency(freq);
  saveSoon();
}

void App::handleTouch(const TouchEvent& ev) {
  switch (ev.type) {
    case TouchEvent::TabFm:
      selectKind(BandKind::FM);
      break;
    case TouchEvent::TabMw:
      selectKind(BandKind::AM);
      break;
    case TouchEvent::TabSw:
      selectKind(BandKind::SW);
      break;
    case TouchEvent::TabMore:
      goHome();
      break;
    case TouchEvent::MorePick:
      if (ev.bandIndex == 0) {
        enterTuner();
      } else if (ev.bandIndex == 1) {
        enterSstv();
      } else if (ev.bandIndex == 2) {
        enterCw();
      } else if (ev.bandIndex == 3) {
        enterHunt();
      } else {
        enterLog();
      }
      break;
    case TouchEvent::OpenAbout:
      ui_.enterAbout();
      break;
    case TouchEvent::OpenSettings:
      openSettings();
      break;
    case TouchEvent::SettingsPick:
      if (ev.bandIndex == 0) {
        wifiOn_ = !wifiOn_;
        saveSoon();
        openSettings();
        ui_.lockTouch();
        applySettings();
      } else if (ev.bandIndex == 1) {
        pixelOn_ = !pixelOn_;
        applySettings();
        saveSoon();
        openSettings();
        ui_.lockTouch();
      } else if (ev.bandIndex == 2) {
        uint8_t next = static_cast<uint8_t>(brightness_ + FENNEC_BRIGHTNESS_STEP);
        if (next > 100) {
          next = FENNEC_BRIGHTNESS_MIN;
        }
        brightness_ = next;
        applySettings();
        saveSoon();
        openSettings();
        ui_.lockTouch();
      } else {
        ui_.calibrate();
        openSettings(true);
        ui_.lockTouch();
      }
      break;
    case TouchEvent::SstvBack:
      goHome();
      break;
    case TouchEvent::SstvUsb:
      radio_.setSsbUsb(!radio_.ssbUsb());
      lastToolHudMs_ = 0;
      if (sstv_.active()) {
        paintSstvHud();
        sstv_.start();
      }
      break;
    case TouchEvent::SstvSrc:
      toggleSstvMic();
      break;
    case TouchEvent::CwSrc:
      toggleCwMic();
      break;
    case TouchEvent::SstvCycleMode:
      sstv_.cyclePreferMode();
      if (sstv_.active()) {
        sstv_.start();
        applySstvSpeaker();
      }
      lastToolHudMs_ = 0;
      paintSstvHud();
      break;
    case TouchEvent::SstvCall: {
      static const uint16_t kCallF[4] = {3733, 7171, 14230, 21340};
      static const bool kCallUsb[4] = {false, false, true, true};
      const uint8_t i = ev.bandIndex > 3 ? 2 : ev.bandIndex;
      radio_.setSsbUsb(kCallUsb[i]);
      radio_.setFrequency(kCallF[i]);
      lastToolHudMs_ = 0;
      paintSstvHud();
      sstv_.start();
      break;
    }
    case TouchEvent::SstvBfoDown:
      radio_.nudgeBfo(static_cast<int16_t>(-FENNEC_SSTV_BFO_STEP));
      lastToolHudMs_ = 0;
      if (sstv_.active()) {
        paintSstvHud();
      }
      break;
    case TouchEvent::SstvBfoUp:
      radio_.nudgeBfo(FENNEC_SSTV_BFO_STEP);
      lastToolHudMs_ = 0;
      if (sstv_.active()) {
        paintSstvHud();
      }
      break;
    case TouchEvent::SstvReset:
      lastToolHudMs_ = 0;
      if (sstv_.active()) {
        paintSstvHud();
        sstv_.start();
      }
      break;
    case TouchEvent::HuntPeak:
      hunt_.resetPeak();
      break;
    case TouchEvent::CwPitchDown:
      cw_.nudgePitch(-50);
      lastToolHudMs_ = 0;
      break;
    case TouchEvent::CwPitchUp:
      cw_.nudgePitch(50);
      lastToolHudMs_ = 0;
      break;
    case TouchEvent::LogArm:
      logger_.setArmed(!logger_.armed());
      lastToolHudMs_ = 0;
      break;
    case TouchEvent::LogClr:
      logger_.clear();
      lastToolHudMs_ = 0;
      break;
    case TouchEvent::LogPreset:
      logger_.setPreset(ev.bandIndex);
      tuneLogPreset();
      lastToolHudMs_ = 0;
      break;
    case TouchEvent::LogThrDown:
      logger_.nudgeThreshold(-1);
      lastToolHudMs_ = 0;
      break;
    case TouchEvent::LogThrUp:
      logger_.nudgeThreshold(1);
      lastToolHudMs_ = 0;
      break;
    case TouchEvent::SwBand:
      if (ev.bandIndex != radio_.bandIndex()) {
        muteRadio();
        radio_.setBand(ev.bandIndex);
        rememberBand();
        scheduleUnmute(FENNEC_BAND_SWITCH_MUTE_MS);
        saveSoon();
      }
      break;
    case TouchEvent::TuneUp:
      radio_.tuneUp(1);
      if (logger_.active()) {
        logger_.setPreset(kNsPresetCount - 1);
      }
      saveSoon();
      break;
    case TouchEvent::TuneDown:
      radio_.tuneDown(1);
      if (logger_.active()) {
        logger_.setPreset(kNsPresetCount - 1);
      }
      saveSoon();
      break;
    case TouchEvent::Seek:
      handleInput(InputEvent::SeekUp);
      break;
    case TouchEvent::Mute:
      handleInput(InputEvent::MuteToggle);
      break;
    case TouchEvent::VolUp:
      handleInput(InputEvent::VolUp);
      break;
    case TouchEvent::VolDown:
      handleInput(InputEvent::VolDown);
      break;
    default:
      break;
  }
}

void App::handleRemote(const RemoteMsg& msg) {
  switch (msg.cmd) {
    case RemoteCmd::TuneUp:
      wakeTuner();
      handleInput(InputEvent::TuneUp);
      break;
    case RemoteCmd::TuneDown:
      wakeTuner();
      handleInput(InputEvent::TuneDown);
      break;
    case RemoteCmd::BandNext:
      wakeTuner();
      handleInput(InputEvent::BandNext);
      break;
    case RemoteCmd::BandPrev:
      wakeTuner();
      handleInput(InputEvent::BandPrev);
      break;
    case RemoteCmd::SetBand:
      wakeTuner();
      muteRadio();
      radio_.setBand(static_cast<uint8_t>(msg.arg));
      rememberBand();
      scheduleUnmute(FENNEC_BAND_SWITCH_MUTE_MS);
      saveSoon();
      break;
    case RemoteCmd::SetKind:
      wakeTuner();
      selectKind(static_cast<BandKind>(constrain(msg.arg, 0, 2)));
      break;
    case RemoteCmd::SetFreq: {
      wakeTuner();
      const uint8_t band = static_cast<uint8_t>((msg.arg >> 16) & 0xFF);
      const uint16_t freq = static_cast<uint16_t>(msg.arg & 0xFFFF);
      gotoBandFreq(band, freq);
      break;
    }
    case RemoteCmd::SeekUp:
      wakeTuner();
      handleInput(InputEvent::SeekUp);
      break;
    case RemoteCmd::SeekDown:
      wakeTuner();
      handleInput(InputEvent::SeekDown);
      break;
    case RemoteCmd::VolDelta: {
      wakeTuner();
      int v = static_cast<int>(radio_.volume()) + static_cast<int>(msg.arg);
      if (v < 0) {
        v = 0;
      }
      if (v > 63) {
        v = 63;
      }
      radio_.setVolume(static_cast<uint8_t>(v));
      lastToolHudMs_ = 0;
      saveSoon();
      break;
    }
    case RemoteCmd::SetVol:
      wakeTuner();
      radio_.setVolume(static_cast<uint8_t>(constrain(msg.arg, 0, 63)));
      lastToolHudMs_ = 0;
      saveSoon();
      break;
    case RemoteCmd::MuteToggle:
      wakeTuner();
      handleInput(InputEvent::MuteToggle);
      break;
    case RemoteCmd::SstvToggle:
      if (sstv_.active()) {
        exitTool();
      } else {
        enterSstv();
      }
      break;
    case RemoteCmd::SetMode:
      if (msg.arg <= 0) {
        enterTuner();
      } else if (msg.arg == 1) {
        if (!sstv_.active()) {
          enterSstv();
        }
      } else if (msg.arg == 2) {
        if (!cw_.active()) {
          enterCw();
        }
      } else if (msg.arg == 3) {
        if (!hunt_.active()) {
          enterHunt();
        }
      } else if (msg.arg == 4) {
        if (!logger_.active()) {
          enterLog();
        }
      }
      break;
    case RemoteCmd::UsbToggle:
      radio_.setSsbUsb(!radio_.ssbUsb());
      lastToolHudMs_ = 0;
      if (sstv_.active()) {
        sstv_.start();
        applySstvSpeaker();
      }
      break;
    case RemoteCmd::BfoDelta:
      radio_.nudgeBfo(static_cast<int16_t>(msg.arg));
      lastToolHudMs_ = 0;
      break;
    case RemoteCmd::SstvMic:
      if (!sstv_.active()) {
        enterSstv();
      }
      sstv_.setSource(msg.arg ? SstvSource::Mic : SstvSource::Radio);
      ui_.setSstvMic(sstv_.mic());
      applySstvSpeaker();
      lastToolHudMs_ = 0;
      paintSstvHud();
      break;
    case RemoteCmd::CwMic:
      if (!cw_.active()) {
        enterCw();
      }
      cw_.setSource(msg.arg ? CwSource::Mic : CwSource::Radio);
      applySstvSpeaker();
      lastToolHudMs_ = 0;
      if (ui_.screen() == UiScreen::Cw) {
        ui_.cwDraw(cw_, radio_.frequency(), radio_.ssbUsb(), radio_.bfo(), radio_.volume(), cw_.mic());
      }
      break;
    case RemoteCmd::SstvCycleMode:
      sstv_.cyclePreferMode();
      if (sstv_.active()) {
        sstv_.start();
        applySstvSpeaker();
      } else {
        enterSstv();
      }
      lastToolHudMs_ = 0;
      paintSstvHud();
      break;
    case RemoteCmd::SstvReset:
      if (sstv_.active()) {
        sstv_.start();
        applySstvSpeaker();
      } else {
        enterSstv();
      }
      lastToolHudMs_ = 0;
      paintSstvHud();
      break;
    case RemoteCmd::CwPitchDelta:
      if (!cw_.active()) {
        enterCw();
      }
      cw_.nudgePitch(static_cast<int16_t>(msg.arg));
      lastToolHudMs_ = 0;
      break;
    case RemoteCmd::LogArm:
      if (!logger_.active()) {
        enterLog();
      }
      logger_.setArmed(msg.arg != 0);
      lastToolHudMs_ = 0;
      break;
    case RemoteCmd::LogClear:
      logger_.clear();
      lastToolHudMs_ = 0;
      break;
    case RemoteCmd::LogThrDelta:
      if (!logger_.active()) {
        enterLog();
      }
      logger_.nudgeThreshold(static_cast<int8_t>(msg.arg));
      lastToolHudMs_ = 0;
      break;
    case RemoteCmd::LogPreset:
      if (!logger_.active()) {
        enterLog();
      }
      logger_.setPreset(static_cast<uint8_t>(msg.arg));
      logger_.applyPreset(radio_);
      lastToolHudMs_ = 0;
      break;
    case RemoteCmd::HuntPeak:
      if (!hunt_.active()) {
        enterHunt();
      }
      hunt_.resetPeak();
      lastToolHudMs_ = 0;
      break;
    default:
      break;
  }
}

void App::muteRadio() {
  audio_.enableAmp(false);
  radio_.setSoftMute(true);
}

void App::scheduleUnmute(uint16_t delayMs) {
  if (userMuted_) {
    return;
  }
  unmuteAt_ = millis() + delayMs;
}

void App::pollUnmute() {
  if (unmuteAt_ == 0 || userMuted_) {
    return;
  }
  if (sstv_.active() && sstv_.mic()) {
    unmuteAt_ = 0;
    return;
  }
  if (millis() < unmuteAt_) {
    return;
  }
  unmuteAt_ = 0;
  radio_.setSoftMute(false);
  audio_.enableAmp(true);
}

void App::pollSignal() {
  const uint32_t now = millis();
  if (now - lastSignalMs_ >= FENNEC_SIGNAL_POLL_MS) {
    lastSignalMs_ = now;
    signal_ = radio_.readSignal();
  }
  if (radio_.isFM() && (now - lastRdsMs_ >= FENNEC_RDS_POLL_MS)) {
    lastRdsMs_ = now;
    radio_.pollRds();
  }
}

void App::publish() {
  UiState u;
  u.frequency = radio_.frequency();
  u.bandIndex = radio_.bandIndex();
  u.rssi = signal_.rssi;
  u.snr = signal_.snr;
  u.volume = radio_.volume();
  u.stereo = signal_.stereo;
  u.muted = userMuted_ || radio_.softMuted() || !audio_.ampEnabled();
  u.present = radio_.isPresent();
  u.wifi = web_.ready();
  u.sstv = sstv_.active();
  strncpy(u.rds, radio_.rdsName(), sizeof(u.rds) - 1);
  clock_.format(u.clock, sizeof(u.clock));
  ui_.apply(u);
  updatePixel(u);

  RadioSnapshot s;
  s.frequency = u.frequency;
  s.bandIndex = u.bandIndex;
  s.volume = u.volume;
  s.rssi = hunt_.active() ? hunt_.rssi() : u.rssi;
  s.snr = hunt_.active() ? hunt_.snr() : u.snr;
  s.stereo = u.stereo;
  s.muted = u.muted;
  s.present = u.present;
  s.sstv = u.sstv;
  s.ssb = radio_.ssbOn();
  s.usb = radio_.ssbUsb();
  s.bfo = radio_.bfo();
  s.sstvMic = sstv_.mic();
  {
    const SstvStatus st = sstv_.status();
    s.sstvMode = static_cast<uint8_t>(st.mode);
    s.sstvLock = static_cast<uint8_t>(st.prefer);
    s.sstvLine = st.line;
    s.sstvLines = st.lines;
    s.sstvVis = st.vis;
    s.sstvPeak = st.peak;
    s.sstvTone = st.freqHz;
  }
  s.sstvGen = sstv_.frameGen();
  s.cwPitch = cw_.pitch();
  s.cwWpm = cw_.wpm();
  s.cwMic = cw_.mic();
  s.huntPeak = hunt_.peak();
  s.logArmed = logger_.armed();
  s.logThr = logger_.threshold();
  s.logCount = logger_.count();
  strncpy(s.clock, u.clock, sizeof(s.clock) - 1);
  {
    const char* t = cw_.text();
    size_t n = strlen(t);
    if (n + 1 > sizeof(s.cw)) {
      t += n + 1 - sizeof(s.cw);
    }
    uint8_t j = 0;
    for (; t[j] != 0 && j + 1 < sizeof(s.cw); ++j) {
      const char c = t[j];
      s.cw[j] = (c == '"' || c == '\\' || static_cast<uint8_t>(c) < 32) ? ' ' : c;
    }
    s.cw[j] = 0;
  }
  if (hunt_.active()) {
    strncpy(s.mode, "hunt", sizeof(s.mode) - 1);
  } else if (cw_.active() && !logger_.active()) {
    strncpy(s.mode, "cw", sizeof(s.mode) - 1);
  } else if (logger_.active()) {
    strncpy(s.mode, "log", sizeof(s.mode) - 1);
  } else if (sstv_.active()) {
    strncpy(s.mode, "sstv", sizeof(s.mode) - 1);
  } else {
    strncpy(s.mode, "tuner", sizeof(s.mode) - 1);
  }
  strncpy(s.rds, u.rds, sizeof(s.rds) - 1);
  strncpy(s.ip, web_.ip(), sizeof(s.ip) - 1);
  web_.setSnapshot(s);
  web_.setSstvFrame(sstv_.frame(), kSstvOutW, kSstvOutH);
  web_.setHuntHist(hunt_.history(), hunt_.histCount(), hunt_.histHead());
  logger_.dumpJson(logJson_, sizeof(logJson_));
  web_.setLogJson(logJson_);
}

void App::saveSoon() { saveAt_ = millis() + 1500; }

void App::pollSave() {
  if (saveAt_ == 0 || millis() < saveAt_) {
    return;
  }
  saveAt_ = 0;
  savePrefs();
}

void App::loadPrefs() {
  if (!prefs.begin("fennec", true)) {
    return;
  }
  const uint8_t band = prefs.getUChar("band", 0);
  const uint16_t freq = prefs.getUShort("freq", 0);
  const uint8_t vol = prefs.getUChar("vol", FENNEC_DEFAULT_VOLUME);
  wifiOn_ = prefs.getBool("wifi", true);
  pixelOn_ = prefs.getBool("pixel", true);
  brightness_ = prefs.getUChar("bright", FENNEC_DEFAULT_BRIGHTNESS);
  prefs.end();

  radio_.setVolume(vol);
  radio_.setBand(band);
  if (freq != 0) {
    radio_.setFrequency(freq);
  }
  rememberBand();
}

void App::savePrefs() {
  if (!prefs.begin("fennec", false)) {
    return;
  }
  prefs.putUChar("band", radio_.bandIndex());
  prefs.putUShort("freq", radio_.frequency());
  prefs.putUChar("vol", radio_.volume());
  prefs.putBool("wifi", wifiOn_);
  prefs.putBool("pixel", pixelOn_);
  prefs.putUChar("bright", brightness_);
  prefs.end();
}

void App::enterSstv() {
  captureHome();
  stopExclusive();
  ui_.setSstvMic(sstv_.mic());
  ui_.enterSstv();

  uint16_t f = FENNEC_SSTV_DEFAULT_KHZ;
  bool usb = true;
  if (!radio_.isFM() && radio_.frequency() >= 1710 && radio_.frequency() <= 27900) {
    f = radio_.frequency();
    usb = f >= 10000;
  }

  userMuted_ = false;
  unmuteAt_ = 0;
  radio_.enterSsb(f, usb);
  radio_.setSsbAudioBw(3);
  sstv_.start();
  applySstvSpeaker();
  lastToolHudMs_ = 0;
  paintSstvHud();
}

void App::paintSstvHud() {
  if (!ui_.sstvMode()) {
    return;
  }
  ui_.sstvHud(sstv_.status(), radio_.frequency(), radio_.ssbUsb(), radio_.bfo(), radio_.volume(),
              sstv_.mic());
}

void App::toggleSstvMic() {
  sstv_.setSource(sstv_.mic() ? SstvSource::Radio : SstvSource::Mic);
  ui_.setSstvMic(sstv_.mic());
  applySstvSpeaker();
  lastToolHudMs_ = 0;
  paintSstvHud();
}

void App::toggleCwMic() {
  cw_.setSource(cw_.mic() ? CwSource::Radio : CwSource::Mic);
  applySstvSpeaker();
  lastToolHudMs_ = 0;
  if (ui_.screen() == UiScreen::Cw) {
    ui_.cwDraw(cw_, radio_.frequency(), radio_.ssbUsb(), radio_.bfo(), radio_.volume(), cw_.mic());
  }
}

void App::applySstvSpeaker() {
  if ((sstv_.active() && sstv_.mic()) || (cw_.active() && cw_.mic())) {
    unmuteAt_ = 0;
    radio_.setSoftMute(true);
    audio_.enableAmp(false);
    return;
  }
  if (userMuted_) {
    radio_.setSoftMute(true);
    audio_.enableAmp(false);
    return;
  }
  radio_.setSoftMute(false);
  audio_.enableAmp(true);
}

bool App::toolRadio() const {
  return sstv_.active() || cw_.active() || hunt_.active() || logger_.active();
}

void App::captureHome() {
  if (ui_.screen() == UiScreen::Tuner) {
    savedBand_ = radio_.bandIndex();
    savedFreq_ = radio_.frequency();
  }
}

void App::stopExclusive() {
  sstv_.stop();
  cw_.stop();
  hunt_.stop();
  logger_.stop();
}

void App::goHome() {
  if (ui_.screen() == UiScreen::Tuner) {
    savedBand_ = radio_.bandIndex();
    savedFreq_ = radio_.frequency();
  }
  stopExclusive();
  muteRadio();
  unmuteAt_ = 0;
  const UiScreen from = ui_.screen();
  if (from != UiScreen::Tuner && from != UiScreen::Menu && from != UiScreen::About &&
      from != UiScreen::Settings) {
    radio_.setBand(savedBand_);
    radio_.setFrequency(savedFreq_);
    rememberBand();
  }
  saveSoon();
  ui_.enterMenu();
}

void App::enterTuner() {
  const bool fromTool = toolRadio();
  if (!fromTool && ui_.screen() == UiScreen::Tuner) {
    scheduleUnmute(FENNEC_AMP_UNMUTE_DELAY_MS);
    return;
  }
  if (fromTool) {
    stopExclusive();
    muteRadio();
    radio_.setBand(savedBand_);
    radio_.setFrequency(savedFreq_);
    rememberBand();
  }
  ui_.exitOverlay();
  scheduleUnmute(fromTool ? FENNEC_BAND_SWITCH_MUTE_MS : FENNEC_AMP_UNMUTE_DELAY_MS);
  saveSoon();
}

void App::wakeTuner() {
  if (toolRadio()) {
    return;
  }
  if (ui_.screen() != UiScreen::Tuner) {
    enterTuner();
  }
}

void App::exitTool() {
  goHome();
}

void App::enterCw() {
  captureHome();
  stopExclusive();
  ui_.enterCw();

  uint16_t f = radio_.frequency();
  bool usb = radio_.ssbUsb();
  if (radio_.isFM() || f < 1710 || f > 27900) {
    f = 14000;
    usb = true;
  }
  userMuted_ = false;
  unmuteAt_ = 0;
  radio_.enterSsb(f, usb);
  radio_.setSsbAudioBw(1);
  cw_.start();
  applySstvSpeaker();
  lastToolHudMs_ = 0;
  ui_.cwDraw(cw_, radio_.frequency(), radio_.ssbUsb(), radio_.bfo(), radio_.volume(), cw_.mic());
}

void App::enterHunt() {
  captureHome();
  stopExclusive();
  ui_.enterHunt();
  userMuted_ = false;
  unmuteAt_ = 0;
  radio_.setSoftMute(false);
  audio_.enableAmp(true);
  hunt_.start();
  lastToolHudMs_ = 0;
  ui_.huntDraw(hunt_, radio_.frequency(), radio_.ssbOn(), radio_.ssbUsb());
}

void App::enterLog() {
  captureHome();
  stopExclusive();
  ui_.enterLog();
  logger_.start();
  tuneLogPreset();
  lastToolHudMs_ = 0;
  ui_.logDraw(logger_, radio_.frequency(), radio_.ssbUsb(), radio_.ssbOn());
}

void App::tuneLogPreset() {
  const NsPreset& p = NsLogger::presets()[logger_.preset()];
  if (p.khz == 0) {
    if (cw_.active() && (p.am || !radio_.ssbOn())) {
      cw_.stop();
    }
    return;
  }
  if (p.am) {
    cw_.stop();
    if (radio_.ssbOn() || radio_.isFM()) {
      muteRadio();
      radio_.setBand(firstIndexOf(BandKind::SW));
      scheduleUnmute(FENNEC_BAND_SWITCH_MUTE_MS);
    }
    radio_.setFrequency(p.khz);
    return;
  }
  userMuted_ = false;
  unmuteAt_ = 0;
  radio_.setSoftMute(false);
  audio_.enableAmp(true);
  radio_.enterSsb(p.khz, p.usb);
  radio_.setSsbAudioBw(3);
  if (!cw_.active()) {
    cw_.setSource(CwSource::Radio);
    cw_.start();
  }
}

void App::pollTools() {
  if (sstv_.active()) {
    sstv_.poll();
    SstvLine line;
    while (sstv_.takeLine(&line)) {
      ui_.sstvBlit(line);
    }
  }
  if (cw_.active()) {
    cw_.poll();
  }
  if (logger_.active()) {
    char iso[20];
    clock_.formatIso(iso, sizeof(iso));
    const char* tail = cw_.text();
    const int n = static_cast<int>(strlen(tail));
    if (n > 18) {
      tail += n - 18;
    }
    logger_.poll(radio_, iso, cw_.active() ? tail : "");
  }

  const uint32_t now = millis();
  const uint16_t period = hunt_.active() ? 40 : 250;
  if (now - lastToolHudMs_ < period) {
    return;
  }
  lastToolHudMs_ = now;
  if (sstv_.active()) {
    paintSstvHud();
  }
  if (hunt_.active()) {
    ui_.huntDraw(hunt_, radio_.frequency(), radio_.ssbOn(), radio_.ssbUsb());
  }
  if (cw_.active() && ui_.screen() == UiScreen::Cw) {
    ui_.cwDraw(cw_, radio_.frequency(), radio_.ssbUsb(), radio_.bfo(), radio_.volume(), cw_.mic());
  }
  if (logger_.active()) {
    ui_.logDraw(logger_, radio_.frequency(), radio_.ssbUsb(), radio_.ssbOn());
  }
}

void App::serialPoll() {
  if (Serial.available() <= 0) {
    return;
  }
  const char c = static_cast<char>(Serial.read());
  switch (c) {
    case '?':
      Serial.println(
          F("buttons: UP/DOWN move  OK select  hold OK back  hold UP+DOWN menu"));
      Serial.println(
          F("n/p band  [/] tune  s seek  +/- vol  m mute  t SSTV  v mic  w morse  h homing  l watch  d dump  c cal  i"));
      break;
    case 'n':
      handleInput(InputEvent::BandNext);
      break;
    case 'p':
      handleInput(InputEvent::BandPrev);
      break;
    case ']':
      handleInput(InputEvent::TuneUp);
      break;
    case '[':
      handleInput(InputEvent::TuneDown);
      break;
    case 's':
      handleInput(InputEvent::SeekUp);
      break;
    case '+':
      handleInput(InputEvent::VolUp);
      break;
    case '-':
      handleInput(InputEvent::VolDown);
      break;
    case 'm':
      handleInput(InputEvent::MuteToggle);
      break;
    case 't':
    case 'T':
      if (sstv_.active()) {
        exitTool();
      } else {
        enterSstv();
      }
      break;
    case 'v':
    case 'V':
      if (ui_.sstvMode()) {
        toggleSstvMic();
      } else if (ui_.screen() == UiScreen::Cw) {
        toggleCwMic();
      }
      break;
    case 'w':
    case 'W':
      if (cw_.active() && ui_.screen() == UiScreen::Cw) {
        exitTool();
      } else {
        enterCw();
      }
      break;
    case 'h':
    case 'H':
      if (hunt_.active()) {
        exitTool();
      } else {
        enterHunt();
      }
      break;
    case 'l':
    case 'L':
      if (logger_.active()) {
        exitTool();
      } else {
        enterLog();
      }
      break;
    case 'd':
    case 'D':
      logger_.dumpSerial();
      break;
    case 'c':
    case 'C':
      ui_.calibrate();
      break;
    case 'i': {
      const Band& b = radio_.band();
      Serial.printf("band %s  freq %u  vol %u  rssi %u  snr %u  ip %s\n", b.name, radio_.frequency(),
                    radio_.volume(), signal_.rssi, signal_.snr, web_.ip());
      break;
    }
    default:
      break;
  }
}

void App::updatePixel(const UiState& u) {
  if (hunt_.active()) {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    hunt_.pixel(&r, &g, &b);
    pixel_.setRgb(r, g, b);
    return;
  }
  const UiScreen s = ui_.screen();
  const bool idleFace =
      s == UiScreen::Menu || s == UiScreen::About || s == UiScreen::Settings;
  uint8_t scale = 200;
  if (idleFace) {
    scale = 200;
  } else if (u.muted) {
    scale = 90;
  } else if (u.sstv) {
    scale = 160;
  } else if (cw_.active()) {
    scale = 140;
  } else if (logger_.active()) {
    scale = logger_.armed() ? 200 : 50;
  } else if (!u.present) {
    scale = 0;
  }
  pixel_.setRgb(scale, scale, scale);
}

void App::applySettings() {
  web_.setApEnabled(wifiOn_);
  pixel_.setEnabled(pixelOn_);
  ui_.setBrightness(brightness_);
}

void App::openSettings(bool wipe) {
  ui_.enterSettings(wifiOn_, pixelOn_, brightness_, web_.ip(), wipe);
}

}
