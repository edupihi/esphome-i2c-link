#pragma once

#ifdef USE_ESP_IDF

#include "i2c_slave.h"
#include "esphome/core/component.h"
#include "driver/i2c_types.h"

namespace esphome
{
namespace i2c_slave
{
  class IDFI2CSlave : public I2CSlave, public Component
  {
    public:
      void setup() override;
      void dump_config() override;
      float get_setup_priority() const override { return setup_priority::BUS; }

    protected:
      i2c_port_t port_;
      uint32_t timeout_ = 0;
      bool initialized_ = false;

    private:
      static bool i2c_slave_request_cb_(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_request_event_data_t *evt_data, void *arg);
      static bool i2c_slave_receive_cb_(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_rx_done_event_data_t *evt_data, void *arg);
      static void i2c_slave_task_(void *arg);
  };

} // namespace i2c_slave
} // namespace esphome

#endif // USE_ESP_IDF
