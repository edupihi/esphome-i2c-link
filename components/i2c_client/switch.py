import esphome.codegen as cg
from esphome.components import i2c, switch
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_I2C_ID,
    DEVICE_CLASS_SWITCH,
    # ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_NONE,
)

DEPENDENCIES = ["i2c"] # client depends on i2c (master, extends i2c::I2CDevice)

# CONF_WIFI_SIGNAL = "wifi_signal"
# CONF_UPTIME = "uptime"
CONF_SWITCH = "switch"
ICON_TOGGLE = "mdi:toggle-switch"
CONF_I2C_REG_KEY_TOGGLE = "i2c_registry_key_toggle"
CONF_I2C_REG_KEY_STATE = "i2c_registry_key_state"

i2c_client_ns = cg.esphome_ns.namespace("i2c_client")
I2CClientSwitch = i2c_client_ns.class_("I2CClientSwitch", switch.Switch, cg.Component, i2c.I2CDevice)


CONFIG_SCHEMA = (
    switch.switch_schema(
        I2CClientSwitch,
        device_class=DEVICE_CLASS_SWITCH,
        block_inverted=True,
        icon=ICON_TOGGLE,
        entity_category=ENTITY_CATEGORY_NONE,
    )
    .extend(
    {
        cv.Required(CONF_I2C_REG_KEY_TOGGLE): cv.hex_uint8_t,
        cv.Required(CONF_I2C_REG_KEY_STATE): cv.hex_uint8_t,
    })
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x0))
)

async def to_code(config):
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_registry_key_toggle(config[CONF_I2C_REG_KEY_TOGGLE]))
    cg.add(var.set_registry_key_state(config[CONF_I2C_REG_KEY_STATE]))

