#include <iostream>
#include "i2c_client.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "../i2c/i2c_bus_esp_idf.h"

namespace esphome {
namespace i2c_client {

static const char *const TAG = "i2c_client.switch";

#ifdef I2C_DEBUG_TIMING
uint64_t I2CClientSwitch::timestamp_() {
  uint64_t count;
  gptimer_get_raw_count(this->gptimer, &count);
  return count;
}
#endif // I2C_DEBUG_TIMING

void I2CClientSwitch::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");
  // get_state();

#ifdef I2C_DEBUG_TIMING
  gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    .intr_priority = 0,
    .flags = { .intr_shared = 0 },
  };
  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &(this->gptimer)));
  ESP_ERROR_CHECK(gptimer_enable(this->gptimer));
  ESP_ERROR_CHECK(gptimer_start(this->gptimer));
#endif // I2C_DEBUG_TIMING

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

    if (last_error_ != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "Sensor read failed");
      this->status_set_warning();
      return false;
    }

    this->status_clear_warning();

    ESP_LOGVV(TAG, "Received reg(0x%02X): 0x%02X 0x%02X 0x%02X 0x%02X <==> %.2f", reg_key_state_, buf.value_raw[0], buf.value_raw[1], buf.value_raw[2], buf.value_raw[3], buf.value_fl);

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

    if (last_error_ != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "Sensor read failed");
      this->status_set_warning();
      return false;
    }

    this->status_clear_warning();

    ESP_LOGVV(TAG, "Received reg(0x%02X): 0x%02X 0x%02X 0x%02X 0x%02X <==> %.2f", reg_key_state_, buf.value_raw[0], buf.value_raw[1], buf.value_raw[2], buf.value_raw[3], buf.value_fl);

    return true;
  });

  return true;
}

// Implement write_state from switch_::Switch
void I2CClientSwitch::write_state(bool state) {
#ifdef I2C_DEBUG_TIMING
  uint64_t t[5] = {0};
  uint8_t ti = 0;
  t[ti++] = timestamp_();
  ESP_LOGD(TAG, "[%lld : %4.3f ms] Switch write_state started", t[0], 0.0f);
#endif // I2C_DEBUG_TIMING

  // Take semaphore to ensure that no other sensor/switch is requesting on i2cbus
  esphome::i2c::IDFI2CBus *bus = reinterpret_cast<esphome::i2c::IDFI2CBus *>(this->bus_);
  xSemaphoreTake(bus->semaphore_, SEMAPHORE_TIMEOUT / portTICK_PERIOD_MS);

#ifdef I2C_DEBUG_TIMING
  t[ti++] = timestamp_();
  ESP_LOGVV(TAG,"[%lld : %4.3f ms] Took semaphore(%p): value: %d", t[ti-1], (float)((t[ti-1] - t[ti-2]) / 1000.0), &(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));
#endif // I2C_DEBUG_TIMING

  bool st = false;
  read_remote_state(&st);

#ifdef I2C_DEBUG_TIMING
  t[ti++] = timestamp_();
  ESP_LOGVV(TAG,"[%lld : %4.3f ms] Read remote state (0x%.2X): %d", t[ti-1], (float)((t[ti-1] - t[ti-2]) / 1000.0), &(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_), reg_key_state_, st);
#endif // I2C_DEBUG_TIMING

  this->publish_state(state);

#ifdef I2C_DEBUG_TIMING
  t[ti++] = timestamp_();
  ESP_LOGVV(TAG,"[%lld : %4.3f ms] Published state", t[ti-1], (float)((t[ti-1] - t[ti-2]) / 1000.0));
#endif // I2C_DEBUG_TIMING

  xSemaphoreGive(bus->semaphore_);

#ifdef I2C_DEBUG_TIMING
  t[ti++] = timestamp_();
  ESP_LOGVV(TAG,"[%lld : %4.3f ms] Gave semaphore(%p): value: %d", t[ti-1], (float)((t[ti-1] - t[ti-2]) / 1000.0), &(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));
#endif // I2C_DEBUG_TIMING

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
