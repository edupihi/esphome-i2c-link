#include "i2c.h"
#include "esphome/core/log.h"
#include <memory>

namespace esphome {
namespace i2c {

static const char *const TAG = "i2c";

ErrorCode I2CDevice::read_register(uint8_t a_register, uint8_t *data, size_t len, bool stop) {
  ErrorCode err = this->write(&a_register, 1, stop);
  if (err != ERROR_OK)
    return err;
  return bus_->read(address_, data, len);
}

ErrorCode I2CDevice::read_register16(uint16_t a_register, uint8_t *data, size_t len, bool stop) {
  a_register = convert_big_endian(a_register);
  ErrorCode const err = this->write(reinterpret_cast<const uint8_t *>(&a_register), 2, stop);
  if (err != ERROR_OK)
    return err;
  return bus_->read(address_, data, len);
}

ErrorCode I2CDevice::write_register(uint8_t a_register, const uint8_t *data, size_t len, bool stop) {
  WriteBuffer buffers[2];
  buffers[0].data = &a_register;
  buffers[0].len = 1;
  buffers[1].data = data;
  buffers[1].len = len;
  return bus_->writev(address_, buffers, 2, stop);
}

ErrorCode I2CDevice::write_register16(uint16_t a_register, const uint8_t *data, size_t len, bool stop) {
  a_register = convert_big_endian(a_register);
  WriteBuffer buffers[2];
  buffers[0].data = reinterpret_cast<const uint8_t *>(&a_register);
  buffers[0].len = 2;
  buffers[1].data = data;
  buffers[1].len = len;
  return bus_->writev(address_, buffers, 2, stop);
}

uint8_t I2CRegister::get() const {
  uint8_t value = 0x00;
  this->parent_->read_register(this->register_, &value, 1);
  return value;
}

uint8_t I2CRegister16::get() const {
  uint8_t value = 0x00;
  this->parent_->read_register16(this->register_, &value, 1);
  return value;
}

}  // namespace i2c
}  // namespace esphome
