#include <iostream>
#include "i2c_client.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "../i2c/i2c_bus_esp_idf.h"

namespace esphome {
namespace i2c_client {

static const char *const TAG = "i2c_client.sensor";

#ifdef I2C_DEBUG_TIMING
uint64_t I2CClientSensor::timestamp_() {
  uint64_t count;
  gptimer_get_raw_count(this->gptimer, &count);
  return count;
}
#endif // I2C_DEBUG_TIMING

void I2CClientSensor::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");

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

auto err = this->write(nullptr, 0);
  if (err != i2c::ERROR_OK) {
    this->mark_failed();
    return;
  }

  ESP_LOGV(TAG, "Initialization complete");
}

void I2CClientSensor::update() {

#ifdef I2C_DEBUG_TIMING
  uint64_t t[5] = {0};
  uint8_t ti = 0;
  t[ti++] = timestamp_();
  ESP_LOGVV(TAG, "[%lld : %7.3f ms] Sensor update started", t[0], 0.0f);
#endif // I2C_DEBUG_TIMING

// Synchronize
  esphome::i2c::IDFI2CBus *bus = reinterpret_cast<esphome::i2c::IDFI2CBus *>(this->bus_);
  xSemaphoreTake(bus->semaphore_, SEMAPHORE_TIMEOUT / portTICK_PERIOD_MS);

#ifdef I2C_DEBUG_TIMING
  t[ti++] = timestamp_();
  ESP_LOGVV(TAG,"[%lld : %7.3f ms] Took semaphore(%p): current value: %d", t[ti-1], (float)((t[ti-1] - t[ti-2]) / 1000.0), &(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));
#endif // I2C_DEBUG_TIMING

  // Send command
  last_error_ = this->write((uint8_t *)&reg_key_, 1);
  if (last_error_ != i2c::ERROR_OK) {
    // Warning will be printed only if warning status is not set yet
    this->status_set_warning("Failed to send command");
    return;
  }

#ifdef I2C_DEBUG_TIMING
  t[ti++] = timestamp_();
  ESP_LOGVV(TAG,"[%lld : %7.3f ms] Wrote command(0x%02X)", t[ti-1], (float)((t[ti-1] - t[ti-2]) / 1000.0), reg_key_);
  this->set_timeout(SEMAPHORE_TIMEOUT, [this, bus, t, ti]() {
#else
  this->set_timeout(SEMAPHORE_TIMEOUT, [this, bus]() {
#endif // I2C_DEBUG_TIMING

    value_t buf = { .value_fl = 0.0f };
    last_error_ = this->read((uint8_t *)&(buf.value_raw), 4);

    if (last_error_ != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "Sensor read failed");
      this->status_set_warning();
      return;
    }
    this->status_clear_warning();

#ifdef I2C_DEBUG_TIMING
    uint64_t t0 = timestamp_();
    ESP_LOGVV(TAG, "[%lld : %7.3f ms] Received reg(0x%02X): 0x%02X 0x%02X 0x%02X 0x%02X <==> %.2f", t0, (float)((t0 - t[ti-1]) / 1000.0), reg_key_, buf.value_raw[0], buf.value_raw[1], buf.value_raw[2], buf.value_raw[3], buf.value_fl);
#endif // I2C_DEBUG_TIMING

    // Evaluate and publish measurements
    if (this->sensor_ != nullptr) {
      this->sensor_->publish_state(buf.value_fl);
    }

#ifdef I2C_DEBUG_TIMING
    uint64_t t1 = timestamp_();
    ESP_LOGVV(TAG,"[%lld : %7.3f ms] Published state", t1, (float)((t1 - t0) / 1000.0));
#endif // I2C_DEBUG_TIMING

    // Release seamphore
    xSemaphoreGive(bus->semaphore_);

#ifdef I2C_DEBUG_TIMING
    uint64_t t2 = timestamp_();
    ESP_LOGVV(TAG,"[%lld : %7.3f ms] Gave semaphore(%p): value: %d", t2, (float)((t2 - t1) / 1000.0), &(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));
#endif // I2C_DEBUG_TIMING

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
