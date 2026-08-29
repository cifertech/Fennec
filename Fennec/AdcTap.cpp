#include "AudioTap.h"

#include <esp_timer.h>

#include "Pins.h"

namespace fennec {

AdcTap& AdcTap::instance() {
  static AdcTap tap;
  return tap;
}

void AdcTap::begin() {
  if (inited_) {
    return;
  }
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_SSTV_ADC, ADC_11db);
  pinMode(PIN_SSTV_ADC, INPUT);
  inited_ = true;
  Serial.printf("[adc] GPIO %u @ %u Hz\n", PIN_SSTV_ADC, kFs);
}

bool AdcTap::start() {
  begin();
  stop();
  w_ = 0;
  r_ = 0;
  dc_ = 2048;
  running_ = true;

  esp_timer_create_args_t args = {};
  args.callback = &AdcTap::sampleCb;
  args.arg = this;
  args.dispatch_method = ESP_TIMER_TASK;
  args.name = "adctap";
  esp_timer_handle_t t = nullptr;
  if (esp_timer_create(&args, &t) != ESP_OK || t == nullptr) {
    running_ = false;
    Serial.println(F("[adc] timer create failed"));
    return false;
  }
  timer_ = t;
  if (esp_timer_start_periodic(t, 1000000ULL / kFs) != ESP_OK) {
    esp_timer_delete(t);
    timer_ = nullptr;
    running_ = false;
    Serial.println(F("[adc] timer start failed"));
    return false;
  }
  return true;
}

void AdcTap::stop() {
  running_ = false;
  if (timer_ != nullptr) {
    auto* t = static_cast<esp_timer_handle_t>(timer_);
    esp_timer_stop(t);
    esp_timer_delete(t);
    timer_ = nullptr;
  }
}

void AdcTap::sampleCb(void* arg) {
  auto* self = static_cast<AdcTap*>(arg);
  if (self == nullptr || !self->running_) {
    return;
  }
  const int raw = analogRead(PIN_SSTV_ADC);
  int32_t dc = self->dc_;
  dc += (raw - dc) >> 6;
  self->dc_ = dc;
  const int16_t s = static_cast<int16_t>(raw - dc);
  self->push(s);
}

void AdcTap::push(int16_t s) {
  const uint16_t n = static_cast<uint16_t>((w_ + 1) % kRing);
  if (n == r_) {
    return;
  }
  ring_[w_] = s;
  w_ = n;
}

bool AdcTap::pop(int16_t* s) {
  if (r_ == w_ || s == nullptr) {
    return false;
  }
  *s = ring_[r_];
  r_ = static_cast<uint16_t>((r_ + 1) % kRing);
  return true;
}

}
