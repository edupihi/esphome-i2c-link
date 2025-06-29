#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/helpers.h"

#include <vector>

namespace esphome {
namespace i2c_client {

typedef union value_u {
  float value_fl;
  uint8_t value_raw[4];
} value_t;

class I2CClientComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  enum CommandLen : uint8_t { ADDR_8_BIT = 1, ADDR_16_BIT = 2 };

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; };

  void set_registry_key(uint8_t key) { reg_key_ = key; };

  void set_sensor(sensor::Sensor *sensor) { sensor_ = sensor; };

    /** Write a command to the i2c device.
   * @param command i2c command to send
   * @return true if reading succeeded
   */
  template<class T> bool write_command(T i2c_register) { return write_command(i2c_register, nullptr, 0); }

  /** Write a command and one data word to the i2c device .
   * @param command i2c command to send
   * @param data argument for the i2c command
   * @return true if reading succeeded
   */
  template<class T> bool write_command(T i2c_register, uint16_t data) { return write_command(i2c_register, &data, 1); }

  /** Write a command with arguments as words
   * @param i2c_register i2c command to send - an be uint8_t or uint16_t
   * @param data vector<uint16> arguments for the i2c command
   * @return true if reading succeeded
   */
  template<class T> bool write_command(T i2c_register, const std::vector<uint16_t> &data) {
    return write_command_(i2c_register, sizeof(T), data.data(), data.size());
  }

  /** Write a command with arguments as words
   * @param i2c_register i2c command to send - an be uint8_t or uint16_t
   * @param data arguments for the i2c command
   * @param len number of arguments (words)
   * @return true if reading succeeded
   */
  template<class T> bool write_command(T i2c_register, const uint16_t *data, uint8_t len) {
    // limit to 8 or 16 bit only
    static_assert(sizeof(i2c_register) == 1 || sizeof(i2c_register) == 2,
                  "only 8 or 16 bit command types are supported.");
    return write_command_(i2c_register, CommandLen(sizeof(T)), data, len);
  }


 protected:
  /** Write a command with arguments as words
   * @param command i2c command to send can be uint8_t or uint16_t
   * @param command_len either 1 for short 8 bit command or 2 for 16 bit command codes
   * @param data arguments for the i2c command
   * @param data_len number of arguments (words)
   * @return true if reading succeeded
   */
  bool write_command_(uint8_t command, CommandLen command_len, const uint16_t *data, uint8_t data_len);

  uint8_t reg_key_{0x0};
  sensor::Sensor *sensor_{nullptr};

  /** last error code from i2c operation
   */
  i2c::ErrorCode last_error_;

};

}  // namespace i2c_client
}  // namespace esphome
