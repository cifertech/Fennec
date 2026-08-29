#pragma once

#include "CwDecoder.h"
#include "DisplayUI.h"
#include "HuntMode.h"
#include "NsLogger.h"
#include "Periph.h"
#include "Radio.h"
#include "SstvDecoder.h"
#include "WebRemote.h"

namespace fennec {

class App {
 public:
  void begin();
  void loop();

 private:
  void handleInput(InputEvent e);
  void handleMenuButtons(InputEvent e);
  void handleSettingsButtons(InputEvent e);
  void handleFocusButtons(InputEvent e);
  void handleTouch(const TouchEvent& ev);
  void handleRemote(const RemoteMsg& msg);
  void selectKind(BandKind kind);
  void rememberBand();
  void gotoBandFreq(uint8_t index, uint16_t freq);
  void muteRadio();
  void scheduleUnmute(uint16_t delayMs);
  void pollUnmute();
  void pollSignal();
  void publish();
  void saveSoon();
  void pollSave();
  void loadPrefs();
  void savePrefs();
  void serialPoll();
  void captureHome();
  void stopExclusive();
  void goHome();
  void enterTuner();
  void wakeTuner();
  void exitTool();
  void enterSstv();
  void paintSstvHud();
  void toggleSstvMic();
  void toggleCwMic();
  void applySstvSpeaker();
  void enterCw();
  void enterHunt();
  void enterLog();
  void tuneLogPreset();
  void pollTools();
  void updatePixel(const UiState& u);
  bool toolRadio() const;
  void applySettings();
  void openSettings(bool wipe = false);

  Radio radio_;
  AudioCtrl audio_;
  DisplayUI ui_;
  Input input_;
  WebRemote web_;
  SstvDecoder sstv_;
  CwDecoder cw_;
  HuntMode hunt_;
  NsLogger logger_;
  ClockAlarm clock_;
  StatusLed pixel_;

  SignalQuality signal_{};
  uint32_t lastSignalMs_ = 0;
  uint32_t lastRdsMs_ = 0;
  uint32_t unmuteAt_ = 0;
  uint32_t saveAt_ = 0;
  bool userMuted_ = false;
  bool staNtpArmed_ = false;
  uint8_t lastKind_[3] = {0, 0, 0};
  uint8_t savedBand_ = 0;
  uint16_t savedFreq_ = 0;
  uint32_t lastToolHudMs_ = 0;
  char logJson_[2048] = "{\"ok\":true,\"events\":[]}";
  bool wifiOn_ = true;
  bool pixelOn_ = true;
  uint8_t brightness_ = 100;
};

}
