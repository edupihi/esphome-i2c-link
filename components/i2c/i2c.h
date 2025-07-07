#pragma once

#include "i2c_bus.h"
#include "esphome/core/helpers.h"
#include "esphome/core/optional.h"
#include <array>
#include <vector>

namespace esphome {
namespace i2c {

#define LOG_I2C_DEVICE(this) ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);

class I2CDevice;  // forward declaration

/// @brief This class is used to create I2CRegister objects that act as proxies to read/write internal registers on an
/// I2C device.
/// @details
/// @n typical usage:
/// @code
/// constexpr uint8_t ADDR_REGISTER_1 = 0x12;
/// i2c::I2CRegister reg_1 = this->reg(ADDR_REGISTER_1); // declare
/// reg_1 |= 0x01; // set bit
/// reg_1 &= ~0x01; // reset bit
/// reg_1 = 10; // Set value
/// uint val = reg_1.get(); // get value
/// @endcode
/// @details The I²C protocol specifies how to read/write in sets of 8-bits followed by an Acknowledgement (ACK/NACK)
/// from the device receiving the data. How the device interprets the bits read/written can vary greatly from
/// device to device. However most of the devices follow the same protocol for reading/writing 8 bit registers using as
/// implemented in the I2CRegister: after sending the device address, the controller sends one byte with the internal
/// register address and then read or write the specified register content.
class I2CRegister {
 public:

  /// @brief returns the register value
  /// @return the register value
  uint8_t get() const;

 protected:
  friend class I2CDevice;

  /// @brief protected constructor that stores the owning object and the register address. Note as only friends can
  /// create an I2CRegister @see I2CDevice::reg()
  /// @param parent our parent
  /// @param a_register address of the i2c register
  I2CRegister(I2CDevice *parent, uint8_t a_register) : parent_(parent), register_(a_register) {}

  I2CDevice *parent_;  ///< I2CDevice object pointer
  uint8_t register_;   ///< the internal address of the register
};

/// @brief This class is used to create I2CRegister16 objects that act as proxies to read/write internal registers
/// (specified with a 16 bit address) on an I2C device.
/// @details
/// @n typical usage:
/// @code
/// constexpr uint16_t X16_BIT_ADDR_REGISTER_1 = 0x1234;
/// i2c::I2CRegister16 reg_1 = this->reg16(X16_BIT_ADDR_REGISTER_1); // declare
/// reg_1 |= 0x01; // set bit
/// reg_1 &= ~0x01; // reset bit
/// reg_1 = 10; // Set value
/// uint val = reg_1.get(); // get value
/// @endcode
/// @details The I²C protocol specification, reads/writes in sets of 8-bits followed by an Acknowledgement (ACK/NACK)
/// from the device receiving the data. How the device interprets the bits read/written to it can vary greatly from
/// device to device. This class can be used to access in the device 8 bits registers that uses a 16 bits internal
/// address. After sending the device address, the controller sends the internal register address (using two consecutive
/// bytes following the big indian convention) and then read or write the register content.
class I2CRegister16 {
 public:

  /// @brief returns the register value
  /// @return the register value
  uint8_t get() const;

 protected:
  friend class I2CDevice;

  /// @brief protected constructor that store the owning object and the register address. Only friends can create an
  /// I2CRegister16 @see I2CDevice::reg16()
  /// @param parent our parent
  /// @param a_register 16 bits address of the i2c register
  I2CRegister16(I2CDevice *parent, uint16_t a_register) : parent_(parent), register_(a_register) {}

  I2CDevice *parent_;  ///< I2CDevice object pointer
  uint16_t register_;  ///< the internal 16 bits address of the register
};

// like ntohs/htons but without including networking headers.
// ("i2c" byte order is big-endian)
inline uint16_t i2ctohs(uint16_t i2cshort) { return convert_big_endian(i2cshort); }
inline uint16_t htoi2cs(uint16_t hostshort) { return convert_big_endian(hostshort); }

/// @brief This Class provides the methods to read/write bytes from/to an i2c device.
/// Objects keep a list of devices found on bus as well as a pointer to the I2CBus in use.
class I2CDevice {
 public:
  /// @brief we use the C++ default constructor
  I2CDevice() = default;

  /// @brief We store the address of the device on the bus
  /// @param address of the device
  void set_i2c_address(uint8_t address) { address_ = address; }

  /// @brief Returns the I2C address of the object.
  /// @return the I2C address
  uint8_t get_i2c_address() const { return this->address_; }

  /// @brief we store the pointer to the I2CBus to use
  /// @param bus pointer to the I2CBus object
  void set_i2c_bus(I2CBus *bus) { bus_ = bus; }

  /// @brief reads an array of bytes from the device using an I2CBus
  /// @param data pointer to an array to store the bytes
  /// @param len length of the buffer = number of bytes to read
  /// @return an i2c::ErrorCode
  ErrorCode read(uint8_t *data, size_t len) { return bus_->read(address_, data, len); }

  /// @brief reads an array of bytes from a specific register in the I²C device
  /// @param a_register an 8 bits internal address of the I²C register to read from
  /// @param data pointer to an array to store the bytes
  /// @param len length of the buffer = number of bytes to read
  /// @param stop (true/false): True will send a stop message, releasing the bus after
  /// transmission. False will send a restart, keeping the connection active.
  /// @return an i2c::ErrorCode
  ErrorCode read_register(uint8_t a_register, uint8_t *data, size_t len, bool stop = true);

  /// @brief reads an array of bytes from a specific register in the I²C device
  /// @param a_register the 16 bits internal address of the I²C register to read from
  /// @param data pointer to an array of bytes to store the information
  /// @param len length of the buffer = number of bytes to read
  /// @param stop (true/false): True will send a stop message, releasing the bus after
  /// transmission. False will send a restart, keeping the connection active.
  /// @return an i2c::ErrorCode
  ErrorCode read_register16(uint16_t a_register, uint8_t *data, size_t len, bool stop = true);

  /// @brief writes an array of bytes to a device using an I2CBus
  /// @param data pointer to an array that contains the bytes to send
  /// @param len length of the buffer = number of bytes to write
  /// @param stop (true/false): True will send a stop message, releasing the bus after
  /// transmission. False will send a restart, keeping the connection active.
  /// @return an i2c::ErrorCode
  ErrorCode write(const uint8_t *data, size_t len, bool stop = true) { return bus_->write(address_, data, len, stop); }

  /// @brief writes an array of bytes to a specific register in the I²C device
  /// @param a_register the internal address of the register to read from
  /// @param data pointer to an array to store the bytes
  /// @param len length of the buffer = number of bytes to read
  /// @param stop (true/false): True will send a stop message, releasing the bus after
  /// transmission. False will send a restart, keeping the connection active.
  /// @return an i2c::ErrorCode
  ErrorCode write_register(uint8_t a_register, const uint8_t *data, size_t len, bool stop = true);

  /// @brief write an array of bytes to a specific register in the I²C device
  /// @param a_register the 16 bits internal address of the register to read from
  /// @param data pointer to an array to store the bytes
  /// @param len length of the buffer = number of bytes to read
  /// @param stop (true/false): True will send a stop message, releasing the bus after
  /// transmission. False will send a restart, keeping the connection active.
  /// @return an i2c::ErrorCode
  ErrorCode write_register16(uint16_t a_register, const uint8_t *data, size_t len, bool stop = true);

  ///
  /// Compat APIs
  /// All methods below have been added for compatibility reasons. They do not bring any functionality and therefore on
  /// new code it is not recommend to use them.
  ///

  bool read_byte(uint8_t a_register, uint8_t *data, bool stop = true) {
    return read_register(a_register, data, 1, stop) == ERROR_OK;
  }

 protected:
  uint8_t address_{0x00};  ///< store the address of the device on the bus
  I2CBus *bus_{nullptr};   ///< pointer to I2CBus instance
};

}  // namespace i2c
}  // namespace esphome
