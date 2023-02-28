#pragma once

#include "esphome.h"

class RelaisSwitch : public Switch {
 public:
  void write_state(bool state) override {
    ESP_LOGD("relais_switch", "write_state");
    // Acknowledge new state by publishing it
    publish_state(state);
  }
};