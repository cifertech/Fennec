#pragma once

#include <Arduino.h>
#include "Bands.h"
#include "CwDecoder.h"
#include "HuntMode.h"
#include "NsLogger.h"
#include "SstvDecoder.h"

namespace fennec {

struct UiState {
  uint16_t frequency = 0;
  uint8_t bandIndex = 0;
  uint8_t rssi = 0;
  uint8_t snr = 0;
  uint8_t volume = 0;
  bool stereo = false;
  bool muted = false;
  bool present = false;
  bool wifi = false;
  bool sstv = false;
  char rds[9] = {0};
  char clock[8] = {0};
};

struct TouchEvent {
  enum Type : uint8_t {
    None = 0,
    TabFm,
    TabMw,
    TabSw,
    TabMore,
    SwBand,
    TuneUp,
    TuneDown,
    Seek,
    Mute,
    VolUp,
    VolDown,
    SstvBack,
    SstvUsb,
    SstvSrc,
    SstvCycleMode,
    SstvCall,
    SstvBfoDown,
    SstvBfoUp,
    SstvReset,
    MorePick,
    HuntPeak,
    CwPitchDown,
    CwPitchUp,
    CwSrc,
    LogArm,
    LogClr,
    LogPreset,
    LogThrDown,
    LogThrUp,
    OpenAbout,
    OpenSettings,
    SettingsPick
  };
  Type type;
  uint8_t bandIndex;
};

enum class UiScreen : uint8_t { Menu, Tuner, Sstv, Hunt, Cw, Log, About, Settings };

class DisplayUI {
 public:
  void begin();
  void showSplash();
  void apply(const UiState& state);
  void poll();
  bool pollTouch(TouchEvent& ev);
  void calibrate();

  UiScreen screen() const { return screen_; }

  void enterSstv();
  void enterMenu();
  void enterHunt();
  void enterCw();
  void enterLog();
  void enterAbout();
  void enterSettings(bool wifiOn, bool pixelOn, uint8_t brightness, const char* ip, bool wipe = false);
  void setBrightness(uint8_t pct);
  void lockTouch();
  void exitOverlay();
  bool sstvMode() const { return screen_ == UiScreen::Sstv; }
  void sstvBlit(const SstvLine& line);
  void sstvHud(const SstvStatus& st, uint16_t freqKhz, bool usb, int16_t bfo, uint8_t volume,
              bool mic);
  void setSstvMic(bool mic);
  void huntDraw(const HuntMode& h, uint16_t freq, bool ssb, bool usb);
  void cwDraw(const CwDecoder& cw, uint16_t freq, bool usb, int16_t bfo, uint8_t volume, bool mic);
  void logDraw(const NsLogger& log, uint16_t freq, bool usb, bool ssb);

  void nudgeBtnFocus(int8_t dir);
  TouchEvent focusEvent() const;
  void refreshChrome();

 private:
  enum Dirty : uint16_t {
    kFull = 1 << 0,
    kFreq = 1 << 1,
    kMeter = 1 << 2,
    kVol = 1 << 3,
    kHead = 1 << 4,
    kSection = 1 << 5,
    kKeys = 1 << 6,
    kDial = 1 << 7,
  };

  void drawFull();
  void drawHeader();
  void drawTabs();
  void drawSection();
  void drawFrequency();
  void drawDial();
  void drawMeters();
  void drawKeys();
  void redrawTouchChrome();
  void drawSstvChrome();
  void drawSstvKeys();
  void drawMenu();
  void drawAbout();
  void drawSettings();
  void paintSettingsRows();
  void paintSettingsIp();
  void formatFreq(char* out, size_t n) const;
  bool noSignal() const;
  TouchEvent::Type hitAt(int16_t x, int16_t y, uint8_t* bandOut) const;
  uint8_t btnFocusCount() const;
  bool btnLit(TouchEvent::Type t, uint8_t slot = 0) const;
  int8_t btnFocusSlot(TouchEvent::Type t) const;

  UiState cur_{};
  UiState last_{};
  uint16_t dirty_ = kFull;
  bool ready_ = false;
  bool touchDown_ = false;
  UiScreen screen_ = UiScreen::Menu;
  bool sstvUsb_ = true;
  bool sstvMic_ = false;
  SstvMode sstvShown_ = SstvMode::Unknown;
  SstvMode sstvPrefer_ = SstvMode::Unknown;
  uint8_t sstvSlot_ = 2;
  int16_t sstvBfo_ = 0;
  uint8_t sstvPressSlot_ = 0;
  bool logArmed_ = false;
  bool logUsb_ = false;
  bool logSsb_ = false;
  uint8_t logPreset_ = 255;
  uint32_t touchRepeatMs_ = 0;
  bool toolChrome_ = false;
  TouchEvent::Type heldType_ = TouchEvent::None;
  bool settingsWifi_ = true;
  bool settingsPixel_ = true;
  uint8_t brightness_ = 100;
  uint8_t settingsBright_ = 100;
  char settingsIp_[16] = "0.0.0.0";
  bool eatTouch_ = false;
  uint32_t touchLockUntil_ = 0;
  uint8_t btnFocus_ = 0;
};

}
