#include <iostream>
#include "i2c_client.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "../i2c/i2c_bus_esp_idf.h"

namespace esphome {
namespace i2c_client {

static const char *const TAG = "i2c_client.switch";

void I2CClientSwitch::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  // get_state();
  ESP_LOGV(TAG, "Initialization complete");
}

bool I2CClientSwitch::read_remote_state(bool *st) {

  last_error_ = this->write((uint8_t *)&reg_key_state_, 1);
  if (last_error_ != i2c::ERROR_OK) {
    // Warning will be printed only if warning status is not set yet
    this->status_set_warning("Failed to send command");
    return false;
  }

  this->set_timeout(10, [this]() {
    value_t buf = { .value_fl = 0.0f };

    last_error_ = this->read((uint8_t *)&(buf.value_raw), 4);

    // xSemaphoreGive(this->semaphore_);

    if (last_error_ != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "Sensor read failed");
      this->status_set_warning();
      return false;
    }

    this->status_clear_warning();

    ESP_LOGVV(TAG, "Receiving reg(0x%02X): 0x%02X 0x%02X 0x%02X 0x%02X", reg_key_state_, buf.value_raw[0], buf.value_raw[1], buf.value_raw[2], buf.value_raw[3]);
    ESP_LOGVV(TAG, "Receiving reg(0x%02X): %.2f", reg_key_state_, buf.value_fl);

    return true;
  });

  return true;
}

bool I2CClientSwitch::write_remote_state(bool st) {

  last_error_ = this->write((uint8_t *)&reg_key_state_, 1);
  if (last_error_ != i2c::ERROR_OK) {
    // Warning will be printed only if warning status is not set yet
    this->status_set_warning("Failed to send command");
    return false;
  }

  this->set_timeout(10, [this]() {
    value_t buf = { .value_fl = 0.0f };

    last_error_ = this->read((uint8_t *)&(buf.value_raw), 4);

    // xSemaphoreGive(this->semaphore_);

    if (last_error_ != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "Sensor read failed");
      this->status_set_warning();
      return false;
    }

    this->status_clear_warning();

    ESP_LOGVV(TAG, "Receiving reg(0x%02X): 0x%02X 0x%02X 0x%02X 0x%02X", reg_key_state_, buf.value_raw[0], buf.value_raw[1], buf.value_raw[2], buf.value_raw[3]);
    ESP_LOGVV(TAG, "Receiving reg(0x%02X): %.2f", reg_key_state_, buf.value_fl);

    return true;
  });

  return true;
}


// Implement write_state from switch_::Switch
void I2CClientSwitch::write_state(bool state) {
  // Take semaphore to ensure that no other sensor/switch is requesting on i2cbus
  esphome::i2c::IDFI2CBus *bus = reinterpret_cast<esphome::i2c::IDFI2CBus *>(this->bus_);
  ESP_LOGVV(TAG,"Taking semaphore(%p): value: %d",&(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));
  xSemaphoreTake(bus->semaphore_, SEMAPHORE_TIMEOUT / portTICK_PERIOD_MS);
  ESP_LOGVV(TAG,"Taken  semaphore(%p): value: %d",&(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));

  bool st = false;
  read_remote_state(&st);

  this->publish_state(state);

  ESP_LOGVV(TAG,"Giving semaphore(%p): value: %d",&(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));
  xSemaphoreGive(bus->semaphore_);
  ESP_LOGVV(TAG,"Given  semaphore(%p): value: %d",&(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));

}

void I2CClientSwitch::dump_config() {
  ESP_LOGCONFIG(TAG, "I2C Client Switch:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  ESP_LOGCONFIG(TAG, "   Switch state: %s.", this->state  ? "ON" : "OFF");
  LOG_SWITCH("", "   Switch", this);
}

}  // namespace i2c_client
}  // namespace esphome
