#pragma once

#include <Arduino.h>
#include "Bands.h"

namespace fennec {

struct RadioSnapshot {
  uint16_t frequency = 0;
  uint8_t bandIndex = 0;
  uint8_t volume = 0;
  uint8_t rssi = 0;
  uint8_t snr = 0;
  bool stereo = false;
  bool muted = false;
  bool present = false;
  bool sstv = false;
  bool ssb = false;
  bool usb = true;
  bool sstvMic = false;
  int16_t bfo = 0;
  uint8_t sstvMode = 0;
  uint8_t sstvLock = 0;
  uint16_t sstvLine = 0;
  uint16_t sstvLines = 0;
  uint16_t sstvVis = 0;
  uint16_t sstvPeak = 0;
  uint16_t sstvTone = 0;
  uint32_t sstvGen = 0;
  uint16_t cwPitch = 700;
  uint8_t cwWpm = 0;
  bool cwMic = false;
  bool hunt = false;
  uint8_t huntPeak = 0;
  bool logArmed = false;
  uint8_t logThr = 0;
  uint8_t logCount = 0;
  char mode[8] = "tuner";
  char rds[9] = {0};
  char ip[16] = "0.0.0.0";
  char clock[8] = {0};
  char cw[96] = {0};
};

enum class RemoteCmd : uint8_t {
  None = 0,
  TuneUp,
  TuneDown,
  BandNext,
  BandPrev,
  SetBand,
  SetKind,
  SetFreq,
  SeekUp,
  SeekDown,
  VolDelta,
  SetVol,
  MuteToggle,
  SstvToggle,
  SetMode,
  UsbToggle,
  BfoDelta,
  SstvMic,
  SstvCycleMode,
  SstvReset,
  CwPitchDelta,
  CwMic,
  LogArm,
  LogClear,
  LogThrDelta,
  LogPreset,
  HuntPeak,
};

struct RemoteMsg {
  RemoteCmd cmd = RemoteCmd::None;
  int32_t arg = 0;
};

class WebRemote {
 public:
  void begin();
  void poll();
  void setSnapshot(const RadioSnapshot& snap);
  void setLogJson(const char* json);
  void setSstvFrame(const uint16_t* px, uint16_t w, uint16_t h);
  void setHuntHist(const uint8_t* hist, uint8_t n, uint8_t head);
  bool take(RemoteMsg& msg);
  bool ready() const { return ready_; }
  bool staUp() const { return staUp_; }
  const char* ip() const { return ip_; }
  void setApEnabled(bool on);
  bool apEnabled() const { return apOn_; }

 private:
  void handleIndex();
  void handleStatus();
  void handleTune();
  void handleBand();
  void handleSeek();
  void handleVolume();
  void handleMute();
  void handleSstv();
  void handleSstvBmp();
  void handleMode();
  void handleUsb();
  void handleBfo();
  void handleCw();
  void handleHunt();
  void handleLog();
  void queue(RemoteCmd cmd, int32_t arg = 0);
  void refreshIp();

  RadioSnapshot snap_{};
  RemoteMsg pending_{};
  char logJson_[2048] = "{\"ok\":true,\"events\":[]}";
  const uint16_t* sstvPx_ = nullptr;
  uint16_t sstvW_ = 0;
  uint16_t sstvH_ = 0;
  const uint8_t* huntHist_ = nullptr;
  uint8_t huntN_ = 0;
  uint8_t huntHead_ = 0;
  bool ready_ = false;
  bool staUp_ = false;
  bool apOn_ = false;
  char ip_[16] = "0.0.0.0";
};

}
