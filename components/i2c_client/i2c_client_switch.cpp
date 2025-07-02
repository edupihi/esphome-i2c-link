#include <iostream>
#include "i2c_client.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace i2c_client {

static const char *const TAG = "i2c_client";

void I2CClientSwitch::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");

  ESP_LOGV(TAG, "Initialization complete");
}

// Implement write_state from switch_::Switch
void I2CClientSwitch::write_state(bool state) {
  // Take semaphore to ensure that no other sensor/switch is requesting
  xSemaphoreTake(this->semaphore_, SEMAPHORE_TIMEOUT / portTICK_PERIOD_MS);

  this->publish_state(state);

  xSemaphoreGive(this->semaphore_);
}

void I2CClientSwitch::dump_config() {
  ESP_LOGCONFIG(TAG, "I2C Client Switch:");
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  ESP_LOGCONFIG(TAG, "   Switch state: %s.", this->switch_->state  ? "ON" : "OFF");

  LOG_SWITCH("", "   Switch", this->switch_);
}

}  // namespace i2c_client
}  // namespace esphome
