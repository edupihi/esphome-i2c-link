#ifdef USE_ESP_IDF

#include "i2c_slave_esp_idf.h"
#include <cinttypes>
#include <cstring>
#include <map>
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <freertos/task.h>
#include "esp_event.h"
#include "driver/i2c_slave.h"

// Command Lists
#define FIRST_COMMAND (0x10)

// #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 3, 0)
// #define SOC_HP_I2C_NUM SOC_I2C_NUM
// #endif

namespace esphome
{
namespace i2c_slave
{
  static const char *const TAG = "i2c_slave.idf";

  typedef struct
  {
    QueueHandle_t event_queue;
    uint8_t command_data;
    i2c_slave_dev_handle_t handle;
    i2c_slave_reg_t *registry;
  } i2c_slave_context_t;

  typedef enum
  {
    I2C_SLAVE_EVT_RX,
    I2C_SLAVE_EVT_TX
  } i2c_slave_event_t;

  void IDFI2CSlave::setup()
  {
    ESP_LOGCONFIG(TAG, "Running setup");

    static i2c_slave_context_t context = (i2c_slave_context_t){ .registry = &registry_ };
    // registry_.insert({ FIRST_COMMAND, 0x12345678 });
    static i2c_port_t next_port = I2C_NUM_0;

    // BUG: can't create new default event loop if f.e. wifi already defines it (see: wifi_component_esp_idf.cpp)
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    // esp_err_t err = esp_event_loop_create_default();
    // if (err != ESP_OK) {
    //   ESP_LOGW(TAG, "i2c_slave_event_loop_creation_failed: %s", esp_err_to_name(err));
    //   this->mark_failed();
    //   return;
    // }

    context.event_queue = xQueueCreate(16, sizeof(i2c_slave_event_t));
    if (!context.event_queue)
    {
      ESP_LOGE(TAG, "Creating queue failed");
      this->mark_failed();
      return;
    }

    ESP_LOGCONFIG(TAG, "i2c_slave_param_config");

    i2c_slave_config_t i2c_slv_config = {
        .i2c_port = next_port,
        .sda_io_num = (gpio_num_t)sda_pin_,
        .scl_io_num = (gpio_num_t)scl_pin_,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .send_buf_depth = 100,
        .receive_buf_depth = 100,
        .slave_addr = address_,
    };

    esp_err_t err = i2c_new_slave_device(&i2c_slv_config, &context.handle);
    if (err != ESP_OK)
    {
      ESP_LOGW(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
      this->mark_failed();
      return;
    }

    ESP_LOGCONFIG(TAG, "i2c_slave_callback_registration");

    i2c_slave_event_callbacks_t cbs = {
        .on_request = i2c_slave_request_cb_,
        .on_receive = i2c_slave_receive_cb_,
    };

    err = i2c_slave_register_event_callbacks(context.handle, &cbs, &context);
    if (err != ESP_OK)
    {
      ESP_LOGW(TAG, "i2c_slave_callback_registration failed: %s", esp_err_to_name(err));
      this->mark_failed();
      return;
    }

    ESP_LOGCONFIG(TAG, "i2c_slave_task_create");

    xTaskCreate(i2c_slave_task_, "i2c_slave_task", 1024 * 4, &context, 10, NULL);

    initialized_ = true;

    ESP_LOGCONFIG(TAG, "Setup successful");
  }

  bool IRAM_ATTR IDFI2CSlave::i2c_slave_request_cb_(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_request_event_data_t *evt_data, void *arg)
  {
    i2c_slave_context_t *context = (i2c_slave_context_t *)arg;
    i2c_slave_event_t evt = I2C_SLAVE_EVT_TX;
    BaseType_t xTaskWoken = 0;
    xQueueSendFromISR(context->event_queue, &evt, &xTaskWoken);
    return xTaskWoken;
  }

  bool IRAM_ATTR IDFI2CSlave::i2c_slave_receive_cb_(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_rx_done_event_data_t *evt_data, void *arg)
  {
    i2c_slave_context_t *context = (i2c_slave_context_t *)arg;
    i2c_slave_event_t evt = I2C_SLAVE_EVT_RX;
    BaseType_t xTaskWoken = 0;
    // Command only contains one byte, so just save one bytes here.
    context->command_data = *evt_data->buffer;
    xQueueSendFromISR(context->event_queue, &evt, &xTaskWoken);
    return xTaskWoken;
  }

  void IDFI2CSlave::i2c_slave_task_(void *arg)
  {
    i2c_slave_context_t *context = (i2c_slave_context_t *)arg;
    i2c_slave_dev_handle_t handle = (i2c_slave_dev_handle_t)context->handle;
    i2c_slave_reg_t *reg;
    i2c_slave_reg_t::iterator reg_it;

    uint8_t zero_buffer[32] = {}; // Use this buffer to clear the fifo.
    uint32_t write_len, total_written;
    uint32_t buffer_size = 0;

    while (true)
    {
      i2c_slave_event_t evt;
      if (xQueueReceive(context->event_queue, &evt, 10) == pdTRUE)
      {
        if (evt == I2C_SLAVE_EVT_TX)
        {
          ESP_LOGW(TAG, "i2c_slave_request_event (RO/TX) received");

          uint8_t *data_buffer;

          reg = context->registry;

          reg_it = reg->find(context->command_data); // lookup requested registry value
          if (reg_it == reg->end()) { // we're past-the-end = not found
            ESP_LOGE(TAG, "Non-existing registry value (0x%02X)requested", context->command_data);
            data_buffer = zero_buffer;
            buffer_size = sizeof(zero_buffer);
          } else {
            // data_buffer = reinterpret_cast<uint8_t *>(&(reg_it->second)); // eg. (int32_t) 0x12345678 => TX: 78563412
            // data_buffer = (uint8_t *)& (reg_it->second);
            // buffer_size = sizeof(reg_it->second);
            ESP_LOGVV(TAG, "Sending reg(0x%02X): %.2f", reg_it->first, (reg_it->second).value_fl);
            ESP_LOGVV(TAG, "Sending reg(0x%02X): 0x%02X 0x%02X 0x%02X 0x%02X", reg_it->first, (reg_it->second).value_raw[0], (reg_it->second).value_raw[1], (reg_it->second).value_raw[2], (reg_it->second).value_raw[3]);
            ESP_LOGVV(TAG, "Sizeof union : %d", sizeof(reg_it->second));
            data_buffer = (reg_it->second).value_raw;
            buffer_size = 4;
          }

          total_written = 0;
          while (total_written < buffer_size)
          {
            ESP_ERROR_CHECK(i2c_slave_write(handle, data_buffer + total_written, buffer_size - total_written, &write_len, 1000));
            if (write_len == 0)
            {
              ESP_LOGE(TAG, "Write error or timeout");
              break;
            }
            total_written += write_len;
          }
        } else if (evt == I2C_SLAVE_EVT_RX) {
          ESP_LOGW(TAG, "i2c_slave_receive_event (RW/RX) received");

          // TODO: something?
        }
      }
    }
    vTaskDelete(NULL);
  }
  void IDFI2CSlave::dump_config()
  {
    ESP_LOGCONFIG(TAG, "I2C SLAVE:");
    ESP_LOGCONFIG(TAG, "  SDA Pin: GPIO%u", this->sda_pin_);
    ESP_LOGCONFIG(TAG, "  SCL Pin: GPIO%u", this->scl_pin_);
    ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
    ESP_LOGCONFIG(TAG, "  Initialized: %u", this->initialized_);
  }

} // namespace i2c_slave
} // namespace esphome

#endif // USE_ESP_IDF
