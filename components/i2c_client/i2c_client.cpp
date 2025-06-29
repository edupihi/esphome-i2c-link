#include <iostream>
#include "i2c_client.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace i2c_client {

static const char *const TAG = "i2c_client";

// To avoid memory allocations for small writes a stack buffer is used
static const size_t BUFFER_STACK_SIZE = 16;

static const uint8_t COMMANDS[] = {0x10, 0x20, 0x30, 0x40}; // STARS, FORKS, OPENISSUES, DESCRIPTIONS

void I2CClientComponent::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");

  // auto err = this->write(nullptr, 0);
  // if (err != i2c::ERROR_OK) {
  //   this->mark_failed();
  //   return;
  // }


  ESP_LOGV(TAG, "Initialization complete");
}

// bool I2CClientComponent::read_data(int32_t *data, uint8_t len) {
//   const uint8_t num_bytes = len;
//   std::vector<uint8_t> buf(num_bytes);

//   last_error_ = this->read(buf.data(), num_bytes);
//   if (last_error_ != i2c::ERROR_OK) {
//     return false;
//   }

//   // for (uint8_t i = 0; i < len; i++) {
//   //   const uint8_t j = 3 * i;
//   //   uint8_t crc = sht_crc_(buf[j], buf[j + 1]);
//   //   if (crc != buf[j + 2]) {
//   //     ESP_LOGE(TAG, "CRC8 Checksum invalid at pos %d! 0x%02X != 0x%02X", i, buf[j + 2], crc);
//   //     last_error_ = i2c::ERROR_CRC;
//   //     return false;
//   //   }
//   //   data[i] = encode_uint16(buf[j], buf[j + 1]);
//   // }
//   *data = encode_uint32(buf[3], buf[2], buf[1], buf[0]);
//   return true;
// }

/***
 * write command with parameters and insert crc
 * use stack array for less than 4 parameters. Most sensirion i2c commands have less parameters
 */
bool I2CClientComponent::write_command_(uint8_t command, CommandLen command_len, const uint16_t *data,
                                        uint8_t data_len) {
  uint8_t temp_stack[BUFFER_STACK_SIZE];
  std::unique_ptr<uint8_t[]> temp_heap;
  uint8_t *temp;
  size_t required_buffer_len = data_len * 3 + 2;

  // Is a dynamic allocation required ?
  if (required_buffer_len >= BUFFER_STACK_SIZE) {
    temp_heap = std::unique_ptr<uint8_t[]>(new uint8_t[required_buffer_len]);
    temp = temp_heap.get();
  } else {
    temp = temp_stack;
  }
  // First byte or word is the command
  uint8_t raw_idx = 0;
  if (command_len == 1) {
    temp[raw_idx++] = command & 0xFF;
  } else {
    // command is 2 bytes
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    temp[raw_idx++] = command >> 8;
    temp[raw_idx++] = command & 0xFF;
#else
    temp[raw_idx++] = command & 0xFF;
    temp[raw_idx++] = command >> 8;
#endif
  }
//   // add parameters followed by crc
//   // skipped if len == 0
//   for (size_t i = 0; i < data_len; i++) {
// #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//     temp[raw_idx++] = data[i] >> 8;
//     temp[raw_idx++] = data[i] & 0xFF;
// #else
//     temp[raw_idx++] = data[i] & 0xFF;
//     temp[raw_idx++] = data[i] >> 8;
// #endif
//     temp[raw_idx++] = sht_crc_(data[i]);
//   }
  last_error_ = this->write(temp, raw_idx);
  return last_error_ == i2c::ERROR_OK;
}

void I2CClientComponent::update() {
  ESP_LOGW(TAG, "Update starting..");
  // Send command
  // if (!this->write_command(COMMANDS[0])) {
  if (!this->write_command_(reg_key_, CommandLen(1), nullptr, 0 )) {
    // Warning will be printed only if warning status is not set yet
    this->status_set_warning("Failed to send command");
    return;
  }

  this->set_timeout(10, [this]() {
    value_t buf = { .value_fl = 0.0f };

    last_error_ = this->read((uint8_t *)&(buf.value_raw), 4);
    if (last_error_ != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "Sensor read failed");
      this->status_set_warning();
      return;
    }

    this->status_clear_warning();

    ESP_LOGVV(TAG, "Receiving reg(0x%02X): 0x%02X 0x%02X 0x%02X 0x%02X", reg_key_, buf.value_raw[0], buf.value_raw[1], buf.value_raw[2], buf.value_raw[3]);
    ESP_LOGVV(TAG, "Receiving reg(0x%02X): %.2f", reg_key_, buf.value_fl);

    // Evaluate and publish measurements
    if (this->sensor_ != nullptr) {
      this->sensor_->publish_state(buf.value_fl);
    }

    // if (this->humidity_sensor_ != nullptr) {
    //   // Relative humidity is in the second result word
    //   float sensor_value_rh = buffer[1];
    //   float rh = -6 + 125 * sensor_value_rh / 65535;

    //   this->humidity_sensor_->publish_state(rh);
    // }
  });
}



void I2CClientComponent::dump_config() {
  if (this->is_failed()) {
    this->setup();
  }
  ESP_LOGCONFIG(TAG, "I2CSlaveDemo:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  // LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  // LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
}

}  // namespace espressif_i2c_demo
}  // namespace esphome
