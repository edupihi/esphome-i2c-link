#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/helpers.h"
#include <vector>

#ifndef I2C_DEBUG_TIMING
#define I2C_DEBUG_TIMING
#include "driver/gptimer.h"
#endif // I2C_DEBUG_TIMING

namespace esphome
{
namespace i2c_client
{
  static const int SEMAPHORE_TIMEOUT = 5; // ms
  typedef union value_u
  {
    float value_fl;
    uint8_t value_raw[4];
  } value_t;

  class I2CClientSensor : public PollingComponent, public i2c::I2CDevice
  {
  public:
    void setup() override;
    void update() override;
    void dump_config() override;
    float get_setup_priority() const override { return setup_priority::DATA; };

    void set_registry_key(uint8_t key) { reg_key_ = key; };

    void set_sensor(sensor::Sensor *sensor) { sensor_ = sensor; };

  protected:
    uint8_t reg_key_{0x0};
    sensor::Sensor *sensor_{nullptr};

    /** last error code from i2c operation
     */
    i2c::ErrorCode last_error_;

#ifdef I2C_DEBUG_TIMING
    uint64_t timestamp_();
    gptimer_handle_t gptimer = NULL;
#endif // I2C_DEBUG_TIMING
  };

  // class I2CClientSwitch : public switch_::Switch, public Component, public i2c::I2CDevice
  class I2CClientSwitch : public switch_::Switch, public PollingComponent, public i2c::I2CDevice
  {
  public:
    void setup() override;
    void dump_config() override;
    void update() override;
    float get_setup_priority() const override { return setup_priority::DATA; };

    void set_registry_key_read(uint8_t key) { reg_key_read_ = key; };
    void set_registry_key_turnon(uint8_t key) { reg_key_turnon_ = key; };
    void set_registry_key_turnoff(uint8_t key) { reg_key_turnoff_ = key; };

    // void set_switch(switch_::Switch *sw) { switch_ = sw; };

  protected:
    void write_state(bool state) override; // this implements write_state(..) from switch_::Switch
    bool request_remote_state(uint8_t *reg_key_, bool *st);
    uint8_t reg_key_read_{0x0};
    uint8_t reg_key_turnon_{0x0};
    uint8_t reg_key_turnoff_{0x0};

    /** last error code from i2c operation
     */
    i2c::ErrorCode last_error_;

#ifdef I2C_DEBUG_TIMING
    uint64_t timestamp_();
    gptimer_handle_t gptimer = NULL;
#endif // I2C_DEBUG_TIMING
  };

} // namespace i2c_client
} // namespace esphome
