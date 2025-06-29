#pragma once
#include <cstdint>
#include <cstddef>
#include <utility>
#include <map>

namespace esphome
{
namespace i2c_slave
{
  // /// @brief Error codes returned by I2CBus and I2CDevice methods
  enum ErrorCode
  {
    NO_ERROR = 0,               ///< No error found during execution of method
    ERROR_OK = 0,               ///< No error found during execution of method
    ERROR_INVALID_ARGUMENT = 1, ///< method called invalid argument(s)
    ERROR_NOT_ACKNOWLEDGED = 2, ///< I2C bus acknowledgment not received
    ERROR_TIMEOUT = 3,          ///< timeout while waiting to receive bytes
    ERROR_NOT_INITIALIZED = 4,  ///< call method to a not initialized bus
    ERROR_TOO_LARGE = 5,        ///< requested a transfer larger than buffers can hold
    ERROR_UNKNOWN = 6,          ///< miscellaneous I2C error during execution
    ERROR_CRC = 7,              ///< bytes received with a CRC error
  };

  typedef union value_u {
    float value_fl;
    uint8_t value_raw[4];
  } value_t;

  typedef std::map<uint8_t, value_t> i2c_slave_reg_t; // key = uint16_t, val = value_t (union)

  /// @brief This Class provides the methods to setup the communication as a single i2c slave address on a bus.
  /// @note The I2CSlave virtual class follows a *Factory design pattern* that provides all the interfaces methods required
  /// by clients while deferring the actual implementation of these methods to subclasses. I2C-specification and
  /// user manual can be found here https://www.nxp.com/docs/en/user-guide/UM10204.pdf and an interesting I²C Application
  /// note https://www.nxp.com/docs/en/application-note/AN10216.pdf
  class I2CSlave
  {
  public:
    /// @brief we use the C++ default constructor
    I2CSlave() = default;

    void set_sda_pin(uint8_t sda_pin) { sda_pin_ = sda_pin; }
    void set_scl_pin(uint8_t scl_pin) { scl_pin_ = scl_pin; }

    /// @brief We store the address of the device on the bus
    /// @param address of the device
    void set_i2c_address(uint8_t address) { address_ = address; }

    /// @brief Returns the I2C address of the object.
    /// @return the I2C address
    uint8_t get_i2c_address() const { return this->address_; }

    void upsert_i2c_registry(uint8_t key, float val) { auto it = registry_.find(key); if (it != registry_.end()) registry_[key].value_fl = val; else registry_.insert({ key, { .value_fl = val } }); };

    float read_i2c_registry(uint8_t key) { auto it = registry_.find(key); if (it != registry_.end()) return registry_[key].value_fl; else return 0.0f; }; // TODO: don't return 0.0 if key not existing

  protected:
    uint8_t sda_pin_;
    uint8_t scl_pin_;
    uint8_t address_{0x00};    ///< store the address of the device on the bus
    i2c_slave_reg_t registry_; // key/value data store for all i2c registers
  };

  /// @brief This Class provides the methods to receive/respond bytes as a single i2c slave device.
  class I2CSlaveDevice
  {
  public:
    /// @brief we use the C++ default constructor
    I2CSlaveDevice() = default;

    /// @brief we store the pointer to the I2C Slave handle to use
    /// @param pointer to the I2C Slave object
    void set_i2c_slave(I2CSlave *slave) { slave_ = slave; }

    /// @brief get the pointer to the I2C Slave object
    I2CSlave *get_i2c_slave() { return slave_; }

  protected:
    I2CSlave *slave_{nullptr}; ///< pointer to I2CSlave instance
  };

} // namespace i2c_slave
} // namespace esphome
