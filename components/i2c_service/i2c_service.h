#pragma once

#include <utility>

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c_slave/i2c_slave.h"

namespace esphome
{
namespace i2c_service
{

  class I2CServiceComponent : public PollingComponent, public i2c_slave::I2CSlaveDevice
  {
  public:
    void setup() override;
    void update() override;
    void dump_config() override;
    float get_setup_priority() const override { return setup_priority::DATA; }

    void set_registry_key(uint8_t key) { reg_key_ = key; }

    /// @brief we store the pointer to the Sensor handle to use
    void set_sensor(sensor::Sensor *sensor) { sensor_ = sensor; }

    /// @brief get the pointer to the Sensor object
    sensor::Sensor *get_sensor() { return sensor_; }

  protected:
    uint8_t reg_key_{0x0};
    sensor::Sensor *sensor_{nullptr}; ///< pointer to I2CSlave instance

  };

} // namespace i2c_service
} // namespace esphome
