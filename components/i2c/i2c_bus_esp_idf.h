#pragma once

// #ifdef USE_ESP_IDF

#include "i2c_bus.h"
#include "esphome/core/component.h"
#include <driver/i2c.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#ifndef I2C_DEBUG_TIMING // MAX 2 GPTIMERS GLOBALLY, OTHERWISE NOT BOOTING
#define I2C_DEBUG_TIMING
#include "driver/gptimer.h"
#endif // I2C_DEBUG_TIMING

namespace esphome {
namespace i2c {

enum RecoveryCode {
  RECOVERY_FAILED_SCL_LOW,
  RECOVERY_FAILED_SDA_LOW,
  RECOVERY_COMPLETED,
};

static const int SEMAPHORE_TIMEOUT = 5; // ms

class IDFI2CBus : public I2CBus, public Component {
 public:
  void setup() override;
  void dump_config() override;
  ErrorCode readv(uint8_t address, ReadBuffer *buffers, size_t cnt) override;
  ErrorCode writev(uint8_t address, WriteBuffer *buffers, size_t cnt, bool stop) override;
  float get_setup_priority() const override { return setup_priority::BUS; }

  void set_scan(bool scan) { scan_ = scan; }
  void set_sda_pin(uint8_t sda_pin) { sda_pin_ = sda_pin; }
  void set_sda_pullup_enabled(bool sda_pullup_enabled) { sda_pullup_enabled_ = sda_pullup_enabled; }
  void set_scl_pin(uint8_t scl_pin) { scl_pin_ = scl_pin; }
  void set_scl_pullup_enabled(bool scl_pullup_enabled) { scl_pullup_enabled_ = scl_pullup_enabled; }
  void set_frequency(uint32_t frequency) { frequency_ = frequency; }
  void set_timeout(uint32_t timeout) { timeout_ = timeout; }

  SemaphoreHandle_t semaphore_;

#ifdef I2C_DEBUG_TIMING
  uint64_t timestamp();
#endif // I2C_DEBUG_TIMING

 private:
  void recover_();
  RecoveryCode recovery_result_;

 protected:
  i2c_port_t port_;
  uint8_t sda_pin_;
  bool sda_pullup_enabled_;
  uint8_t scl_pin_;
  bool scl_pullup_enabled_;
  uint32_t frequency_;
  uint32_t timeout_ = 0;
  bool initialized_ = false;

#ifdef I2C_DEBUG_TIMING
  gptimer_handle_t gptimer = NULL;
#endif // I2C_DEBUG_TIMING

};

}  // namespace i2c
}  // namespace esphome

// #endif  // USE_ESP_IDF
