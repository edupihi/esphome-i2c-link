#include "i2c_service.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace i2c_service {

static const char *const TAG = "i2c_service.switch";


// static class member function, i2c_slave_cb(...)
// as 'static' it has no access to 'this'-object/instance
// instead it gets a pointer to 'this' as parameter
void I2CServiceSwitchComponent::i2c_slave_cb(void *param) {
  I2CServiceSwitchComponent *this_ = (I2CServiceSwitchComponent *)param;

  esphome::i2c_slave::reg_val_t *val = this_->get_i2c_slave()->get_i2c_registry(this_->reg_key_toggle_);

  ESP_LOGVV(TAG, "Callback called, reg: 0x%02X, TOGGLING switch %d -> %d", this_->reg_key_toggle_, this_->switch_->state, !(this_->switch_->state));

  this_->switch_->state == true ? this_->switch_->turn_off() : this_->switch_->turn_on(); // toggle switch

  this_->get_i2c_slave()->upsert_i2c_registry(this_->reg_key_state_, (float) this_->switch_->state);  // update current state in state-registry
  this_->get_i2c_slave()->upsert_i2c_registry(this_->reg_key_toggle_, (float) this_->switch_->state); // update current state in toggle-registry
  ESP_LOGVV(TAG, "Callback called, state  reg: 0x%02X, set to state: %d", this_->reg_key_state_, (bool)this_->get_i2c_slave()->get_i2c_registry(this_->reg_key_state_));
  ESP_LOGVV(TAG, "Callback called, toggle reg: 0x%02X, set to state: %d", this_->reg_key_toggle_, (bool)this_->get_i2c_slave()->get_i2c_registry(this_->reg_key_toggle_));
}

void I2CServiceSwitchComponent::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");

  // set up double registry entries
  // requests for reg_key_state_ is read-only (cmp. sensor)
  // requests for reg_key_toggle_ triggers the callback which toggle the state (and returns new state)
  this->get_i2c_slave()->upsert_i2c_registry(this->reg_key_state_, (float) this->switch_->state); // register current state in state-registry
  this->get_i2c_slave()->upsert_i2c_registry(this->reg_key_toggle_, (float) this->switch_->state); // register current state in toggle-registry

  // register the callback for only for reg_key_toggle_
  this->get_i2c_slave()->set_cb_i2c_registry(this->reg_key_toggle_, &i2c_slave_cb, (void *)this); // register a static member function as callback and a pointer to 'this' object/component

  ESP_LOGV(TAG, "Initialization complete");
}

void I2CServiceSwitchComponent::update() {
  // synchronize switch-state and registry states (if changed locally)
  if (this->switch_->state != (bool)this->get_i2c_slave()->get_i2c_registry(this->reg_key_state_) || (bool)this->get_i2c_slave()->get_i2c_registry(this->reg_key_toggle_)) {
    this->get_i2c_slave()->upsert_i2c_registry(this->reg_key_state_, (float) this->switch_->state);  // update current state in state-registry
    this->get_i2c_slave()->upsert_i2c_registry(this->reg_key_toggle_, (float) this->switch_->state); // update current state in toggle-registry
    ESP_LOGVV(TAG, "Updated registries (0x%02X & 0x%02X) to current switch state: %d", this->reg_key_state_, this->reg_key_toggle_, (float) this->switch_->state);
  }
}

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
