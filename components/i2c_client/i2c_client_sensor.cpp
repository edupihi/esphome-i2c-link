#include <iostream>
#include "i2c_client.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace i2c_client {

static const char *const TAG = "i2c_client";

void I2CClientSensor::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");

  // auto err = this->write(nullptr, 0);
  // if (err != i2c::ERROR_OK) {
  //   this->mark_failed();
  //   return;
  // }

  ESP_LOGV(TAG, "Initialization complete");
}

void I2CClientSensor::update() {
  ESP_LOGW(TAG, "Update starting..");
  // Synchronize
  xSemaphoreTake(this->semaphore_, SEMAPHORE_TIMEOUT / portTICK_PERIOD_MS);
  // Send command
  last_error_ = this->write((uint8_t *)&reg_key_, 1);
  if (last_error_ != i2c::ERROR_OK) {
    // Warning will be printed only if warning status is not set yet
    this->status_set_warning("Failed to send command");
    return;
  }

  this->set_timeout(10, [this]() {
    value_t buf = { .value_fl = 0.0f };

    last_error_ = this->read((uint8_t *)&(buf.value_raw), 4);

    xSemaphoreGive(this->semaphore_);

    if (last_error_ != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "Sensor read failed");
      this->status_set_warning();
      return;
    }

    this->status_clear_warning();

    ESP_LOGVV(TAG, "Receiving reg(0x%02X): 0x%02X 0x%02X 0x%02X 0x%02X", reg_key_, buf.value_raw[0], buf.value_raw[1], buf.value_raw[2], buf.value_raw[3]);
    ESP_LOGVV(TAG, "Receiving reg(0x%02X): %.2f", reg_key_, buf.value_fl);

    // Evaluate and publish measurements
    if (this->sensor_ != nullptr) {
      this->sensor_->publish_state(buf.value_fl);
    }

    // if (this->humidity_sensor_ != nullptr) {
    //   // Relative humidity is in the second result word
    //   float sensor_value_rh = buffer[1];
    //   float rh = -6 + 125 * sensor_value_rh / 65535;

    //   this->humidity_sensor_->publish_state(rh);
    // }
  });
}



void I2CClientSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "I2C Client:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  LOG_SENSOR("  ", "Sensor", this->sensor_);
  // LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
}

}  // namespace i2c_client
}  // namespace esphome
