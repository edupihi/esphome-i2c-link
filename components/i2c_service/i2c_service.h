#pragma once

#include <utility>

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/i2c_slave/i2c_slave.h"

#ifdef ESP_IDF
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif // ESP_IDF

  typedef union value_u
  {
    float value_fl;
    uint8_t value_raw[4];
  } value_t;



namespace esphome
{
namespace i2c_service
{
  static const int SEMAPHORE_TIMEOUT = 5; // ms

  class I2CService : public i2c_slave::I2CSlaveDevice
  {
    public:
    I2CService() { this->semaphore_ = xSemaphoreCreateBinary(); }
    ~I2CService() { vSemaphoreDelete(this->semaphore_); }

    protected:
    SemaphoreHandle_t semaphore_;
  };

  class I2CServiceSensorComponent : public PollingComponent, public I2CService
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

  class I2CServiceSwitchComponent : public Component, public I2CService
  {
  public:
    void setup() override;
    void dump_config() override;
    float get_setup_priority() const override { return setup_priority::DATA; }

    void set_registry_key_toggle(uint8_t key) { reg_key_toggle_ = key; }
    void set_registry_key_state(uint8_t key) { reg_key_state_ = key; }

    /// @brief we store the pointer to the Switch handle to use
    void set_switch(switch_::Switch *sw) { switch_ = sw; }

    /// @brief get the pointer to the Switch object
    switch_::Switch  *get_switch() { return switch_; }

  protected:
    uint8_t reg_key_toggle_{0x0};
    uint8_t reg_key_state_{0x0};
    switch_::Switch *switch_{nullptr}; ///< pointer to I2CSlave instance

  };

} // namespace i2c_service
} // namespace esphome
