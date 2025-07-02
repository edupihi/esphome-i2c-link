#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/helpers.h"

#ifdef ESP_IDF
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif // ESP_IDF

#include <vector>

namespace esphome
{
namespace i2c_client
{

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
    SemaphoreHandle_t semaphore_;
    uint8_t reg_key_{0x0};
    sensor::Sensor *sensor_{nullptr};

    /** last error code from i2c operation
     */
    i2c::ErrorCode last_error_;
  };

  class I2CClientSwitch : public switch_::Switch, public Component, public i2c::I2CDevice
  {
  public:
    // enum CommandLen : uint8_t { ADDR_8_BIT = 1, ADDR_16_BIT = 2 };

    void setup() override;
    void dump_config() override;
    float get_setup_priority() const override { return setup_priority::DATA; };

    void set_registry_key(uint8_t key) { reg_key_ = key; };
    void set_switch(switch_::Switch *sw) { switch_ = sw; };

  protected:
    void write_state(bool state) override;

    uint8_t reg_key_{0x0};

    switch_::Switch *switch_{nullptr};

    /** last error code from i2c operation
     */
    i2c::ErrorCode last_error_;
  };

} // namespace i2c_client
} // namespace esphome
