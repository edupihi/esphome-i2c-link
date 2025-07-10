#include "i2c_service.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace i2c_service {

static const char *const TAG = "i2c_service.switch";

void I2CServiceSwitchComponent::synchronize_registries(I2CServiceSwitchComponent *cmp) {
  cmp->get_i2c_slave()->upsert_i2c_registry(cmp->reg_key_read_, cmp->switch_->state ? 1.0f : 0.0f); // update current state in read-registry
  cmp->get_i2c_slave()->upsert_i2c_registry(cmp->reg_key_turnon_, cmp->switch_->state ? 1.0f : 0.0f); // update current state in turnon-registry
  cmp->get_i2c_slave()->upsert_i2c_registry(cmp->reg_key_turnoff_, cmp->switch_->state ? 1.0f : 0.0f); // update current state in turnoff-registry
  ESP_LOGVV(TAG, "Updated registries (0x%02X, 0x%02X, 0x%02X) to current switch state: %d", cmp->reg_key_read_, cmp->reg_key_turnon_, cmp->reg_key_turnoff_, (bool) cmp->switch_->state);
  // ESP_LOGVV(TAG, "0x%02X = %.2f", cmp->reg_key_read_, cmp->get_i2c_slave()->read_i2c_registry(cmp->reg_key_read_));
  // ESP_LOGVV(TAG, "0x%02X = %.2f", cmp->reg_key_turnon_, cmp->get_i2c_slave()->read_i2c_registry(cmp->reg_key_turnon_));
  // ESP_LOGVV(TAG, "0x%02X = %.2f", cmp->reg_key_turnoff_, cmp->get_i2c_slave()->read_i2c_registry(cmp->reg_key_turnoff_));
}

// static class member function, i2c_slave_cb(...)
// as 'static' it has no access to 'this'-object/instance
// instead it gets a pointer to 'this' as parameter
void I2CServiceSwitchComponent::i2c_slave_cb(uint8_t reg_key, void *param) {
  I2CServiceSwitchComponent *this_ = (I2CServiceSwitchComponent *)param;

  // esphome::i2c_slave::reg_val_t *val = this_->get_i2c_slave()->get_i2c_registry(reg_key);

  if (reg_key == this_->reg_key_turnon_) {
    ESP_LOGVV(TAG, "Callback called, reg: 0x%02X, turning ON switch", reg_key);
    this_->switch_->turn_on();
  } else {
    ESP_LOGVV(TAG, "Callback called, reg: 0x%02X, turning OFF switch", reg_key);
    this_->switch_->turn_off();
  }
  synchronize_registries(this_);
}

void I2CServiceSwitchComponent::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");

  // set up triple registry entries
  synchronize_registries(this);

  // register the callback for reg_key_turnon_ and reg_key_turnoff_
  this->get_i2c_slave()->set_cb_i2c_registry(this->reg_key_turnon_, &i2c_slave_cb, (void *)this); // register a static member function as callback and a pointer to 'this' object/component
  this->get_i2c_slave()->set_cb_i2c_registry(this->reg_key_turnoff_, &i2c_slave_cb, (void *)this); // register a static member function as callback and a pointer to 'this' object/component

  ESP_LOGV(TAG, "Initialization complete");
}

void I2CServiceSwitchComponent::update() {
  // synchronize triple registry entries
  // local switching does not update registries, so we do it with PollingComponent::update() (=delay)
  // ESP_LOGVV(TAG, "Current state = %d, 0x%02X = %.2f", this->switch_->state, this->reg_key_read_, this->get_i2c_slave()->read_i2c_registry(this->reg_key_read_));

  if (this->switch_->state != (bool)(this->get_i2c_slave()->read_i2c_registry(this->reg_key_read_))) {
    synchronize_registries(this);
  }
}

void I2CServiceSwitchComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "I2C Service Switch:");
  ESP_LOGCONFIG(TAG, "  I2C Address: 0x%02X", (this->get_i2c_slave())->get_i2c_address());
  ESP_LOGCONFIG(TAG, "  Registry key (read) : 0x%02X", this->reg_key_read_);
  ESP_LOGCONFIG(TAG, "  Registry key (turnon): 0x%02X", this->reg_key_turnon_);
  ESP_LOGCONFIG(TAG, "  Registry key (turnoff): 0x%02X", this->reg_key_turnoff_);
  ESP_LOGCONFIG(TAG, "  Registry val (state): %.2f", this->get_i2c_slave()->read_i2c_registry(this->reg_key_read_));
  // ESP_LOGCONFIG(TAG, "  Sensor state: %.02f", this->sensor_->state);
}

}  // namespace i2c_service
}  // namespace esphome
