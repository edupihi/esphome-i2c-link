#include "i2c_service.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace i2c_service {

static const char *const TAG = "i2c_service.switch";

void I2CServiceSwitchComponent::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");

  this->get_i2c_slave()->upsert_i2c_registry(this->reg_key_state_, 0.0f); // register 0 as initial value

  ESP_LOGV(TAG, "Initialization complete");
}

// void I2CServiceSwitchComponent::update() {
//   // this->get_i2c_slave()->upsert_i2c_registry(this->reg_key_, this->sensor_->state);
// }

void I2CServiceSwitchComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "I2C Service Switch:");
  ESP_LOGCONFIG(TAG, "  I2C Address: 0x%02X", (this->get_i2c_slave())->get_i2c_address());
  ESP_LOGCONFIG(TAG, "  Registry key (state) : 0x%02X", this->reg_key_state_);
  ESP_LOGCONFIG(TAG, "  Registry key (toggle): 0x%02X", this->reg_key_toggle_);
  ESP_LOGCONFIG(TAG, "  Registry val (state): %.2f", this->get_i2c_slave()->read_i2c_registry(this->reg_key_state_));
  // ESP_LOGCONFIG(TAG, "  Sensor state: %.02f", this->sensor_->state);
}

}  // namespace i2c_service
}  // namespace esphome
