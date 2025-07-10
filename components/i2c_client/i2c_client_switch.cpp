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

  ESP_LOGV(TAG, "Initialization complete");
}

bool I2CClientSwitch::request_remote_state(uint8_t *reg_key, bool *st) {

  esphome::i2c::IDFI2CBus *bus = reinterpret_cast<esphome::i2c::IDFI2CBus *>(this->bus_);

#ifdef I2C_DEBUG_TIMING
  uint64_t t[5] = {0};
  uint8_t ti = 0;
  t[ti++] = bus->timestamp();
  ESP_LOGD(TAG, "[%lld : %4.3f ms] Switch 'request_remote_state' (0x%02X) started", t[0], 0.0f, *reg_key);
#endif // I2C_DEBUG_TIMING

// Take semaphore to ensure that no other sensor/switch is requesting on i2cbus
  xSemaphoreTake(bus->semaphore_, SEMAPHORE_TIMEOUT / portTICK_PERIOD_MS);

#ifdef I2C_DEBUG_TIMING
  t[ti++] = bus->timestamp();
  ESP_LOGVV(TAG,"[%lld : %7.3f ms] Took semaphore(%p): value: %d", t[ti-1], (float)((t[ti-1] - t[ti-2]) / 1000.0), &(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));
#endif // I2C_DEBUG_TIMING

  // last_error_ = this->write((uint8_t *)&reg_key_state_, 1);
  last_error_ = this->write(reg_key, 1);
  if (last_error_ != i2c::ERROR_OK) {
    // Warning will be printed only if warning status is not set yet
    this->status_set_warning("Failed to send command") ;
    return false;
  }

#ifdef I2C_DEBUG_TIMING
  t[ti++] = bus->timestamp();
  ESP_LOGVV(TAG,"[%lld : %7.3f ms] Wrote command(0x%02X)", t[ti-1], (float)((t[ti-1] - t[ti-2]) / 1000.0), *reg_key);
  this->set_timeout(SEMAPHORE_TIMEOUT, [this, bus, reg_key, t, ti]() {
#else
  this->set_timeout(SEMAPHORE_TIMEOUT, [this, bus, reg_key]() {
#endif // I2C_DEBUG_TIMING

    value_t buf = { .value_fl = 0.0f };

    last_error_ = this->read((uint8_t *)&(buf.value_raw), 4);


    if (last_error_ != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "Response read failed");
      this->status_set_warning();
      return false;
    }

    this->status_clear_warning();

#ifdef I2C_DEBUG_TIMING
    uint64_t t0 = bus->timestamp();
    ESP_LOGVV(TAG, "[%lld : %7.3f ms] Received reg(0x%02X): 0x%02X 0x%02X 0x%02X 0x%02X <==> %.2f", t0, (float)((t0 - t[ti-1]) / 1000.0), *reg_key, buf.value_raw[0], buf.value_raw[1], buf.value_raw[2], buf.value_raw[3], buf.value_fl);
#endif // I2C_DEBUG_TIMING

    // Evaluate and publish state
    bool remote_state = (bool)buf.value_fl;
    if (this->state != remote_state) {
      this->state = remote_state;
      this->publish_state(remote_state);
#ifdef I2C_DEBUG_TIMING
      uint64_t t3 = bus->timestamp();
      ESP_LOGVV(TAG, "[%lld : %7.3f ms] Published updated state: %d", t3, (float)((t3 - t0) / 1000.0), remote_state);
#endif // I2C_DEBUG_TIMING
    }

    // Release seamphore
    xSemaphoreGive(bus->semaphore_);

#ifdef I2C_DEBUG_TIMING
    uint64_t t2 = bus->timestamp();
    ESP_LOGVV(TAG,"[%lld : %7.3f ms] Gave semaphore(%p): value: %d", t2, (float)((t2 - t0) / 1000.0), &(bus->semaphore_), uxSemaphoreGetCount(bus->semaphore_));
#endif // I2C_DEBUG_TIMING

    return true;
  });

  return true;
}

// Override write_state(..) from switch_::Switch
void I2CClientSwitch::write_state(bool state) {
  bool st = false;
  // request to turnon/turnoff the remote switch
  state == true ? request_remote_state(&reg_key_turnon_, &st) : request_remote_state(&reg_key_turnoff_, &st);
}

// Override update() from PollingComponent
void I2CClientSwitch::update() {
  bool st = false;
  // request read-reg = read-only state of remote switch
  request_remote_state(&reg_key_read_, &st);
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
