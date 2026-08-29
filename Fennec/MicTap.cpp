#include "AudioTap.h"

#include "Config.h"
#include "Pins.h"

#include <Arduino.h>
#include <esp_idf_version.h>

#if ESP_IDF_VERSION_MAJOR >= 5
#include "driver/i2s_std.h"
#else
#include "driver/i2s.h"
#endif

namespace fennec {

MicTap& MicTap::instance() {
  static MicTap tap;
  return tap;
}

void MicTap::begin() {
  if (inited_) {
    return;
  }
  inited_ = true;
  Serial.printf("[mic] I2S  CK=%u WS=%u SD=%u  %u Hz -> %u Hz\n", PIN_MIC_CK, PIN_MIC_WS, PIN_MIC_SD,
                kHwFs, kFs);
}

bool MicTap::start() {
  begin();
  stop();
  w_ = 0;
  r_ = 0;
  hold_ = 0;
  haveHold_ = false;

#if ESP_IDF_VERSION_MAJOR >= 5
  i2s_chan_handle_t rx = nullptr;
  i2s_chan_config_t chan = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  if (i2s_new_channel(&chan, nullptr, &rx) != ESP_OK || rx == nullptr) {
    Serial.println(F("[mic] i2s channel failed"));
    return false;
  }
  i2s_std_config_t std = {};
  std.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(kHwFs);
  std.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO);
#if FENNEC_MIC_SLOT_RIGHT
  std.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT;
#else
  std.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
#endif
  std.gpio_cfg.mclk = I2S_GPIO_UNUSED;
  std.gpio_cfg.bclk = static_cast<gpio_num_t>(PIN_MIC_CK);
  std.gpio_cfg.ws = static_cast<gpio_num_t>(PIN_MIC_WS);
  std.gpio_cfg.dout = I2S_GPIO_UNUSED;
  std.gpio_cfg.din = static_cast<gpio_num_t>(PIN_MIC_SD);
  std.gpio_cfg.invert_flags.mclk_inv = false;
  std.gpio_cfg.invert_flags.bclk_inv = false;
  std.gpio_cfg.invert_flags.ws_inv = false;
  if (i2s_channel_init_std_mode(rx, &std) != ESP_OK) {
    i2s_del_channel(rx);
    Serial.println(F("[mic] i2s std init failed"));
    return false;
  }
  if (i2s_channel_enable(rx) != ESP_OK) {
    i2s_del_channel(rx);
    Serial.println(F("[mic] i2s enable failed"));
    return false;
  }
  rx_ = rx;
#else
  i2s_config_t cfg = {};
  cfg.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX);
  cfg.sample_rate = kHwFs;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
#if FENNEC_MIC_SLOT_RIGHT
  cfg.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
#else
  cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
#endif
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = 256;
  cfg.use_apll = false;
  cfg.tx_desc_auto_clear = false;
  cfg.fixed_mclk = 0;
  i2s_pin_config_t pins = {};
  pins.bck_io_num = PIN_MIC_CK;
  pins.ws_io_num = PIN_MIC_WS;
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  pins.data_in_num = PIN_MIC_SD;
  if (i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr) != ESP_OK) {
    Serial.println(F("[mic] i2s install failed"));
    return false;
  }
  if (i2s_set_pin(I2S_NUM_0, &pins) != ESP_OK) {
    i2s_driver_uninstall(I2S_NUM_0);
    Serial.println(F("[mic] i2s pins failed"));
    return false;
  }
  rx_ = reinterpret_cast<void*>(1);
#endif

  running_ = true;
  Serial.println(F("[mic] listening"));
  return true;
}

void MicTap::stop() {
  running_ = false;
  if (rx_ == nullptr) {
    return;
  }
#if ESP_IDF_VERSION_MAJOR >= 5
  auto* rx = static_cast<i2s_chan_handle_t>(rx_);
  i2s_channel_disable(rx);
  i2s_del_channel(rx);
#else
  i2s_driver_uninstall(I2S_NUM_0);
#endif
  rx_ = nullptr;
}

void MicTap::poll() {
  if (!running_ || rx_ == nullptr) {
    return;
  }
  int32_t raw[64];
  size_t bytes = 0;
#if ESP_IDF_VERSION_MAJOR >= 5
  auto* rx = static_cast<i2s_chan_handle_t>(rx_);
  if (i2s_channel_read(rx, raw, sizeof(raw), &bytes, 0) != ESP_OK) {
    return;
  }
#else
  if (i2s_read(I2S_NUM_0, raw, sizeof(raw), &bytes, 0) != ESP_OK) {
    return;
  }
#endif
  const size_t n = bytes / sizeof(int32_t);
  size_t i = 0;
  if (haveHold_ && n > 0) {
    const int16_t b = static_cast<int16_t>(raw[0] >> FENNEC_MIC_SHIFT);
    push(static_cast<int16_t>((static_cast<int32_t>(hold_) + b) / 2));
    haveHold_ = false;
    i = 1;
  }
  for (; i + 1 < n; i += 2) {
    const int16_t a = static_cast<int16_t>(raw[i] >> FENNEC_MIC_SHIFT);
    const int16_t b = static_cast<int16_t>(raw[i + 1] >> FENNEC_MIC_SHIFT);
    push(static_cast<int16_t>((static_cast<int32_t>(a) + b) / 2));
  }
  if (i < n) {
    hold_ = static_cast<int16_t>(raw[i] >> FENNEC_MIC_SHIFT);
    haveHold_ = true;
  }
}

void MicTap::push(int16_t s) {
  const uint16_t n = static_cast<uint16_t>((w_ + 1) % kRing);
  if (n == r_) {
    return;
  }
  ring_[w_] = s;
  w_ = n;
}

bool MicTap::pop(int16_t* s) {
  if (r_ == w_ || s == nullptr) {
    return false;
  }
  *s = ring_[r_];
  r_ = static_cast<uint16_t>((r_ + 1) % kRing);
  return true;
}

}
