import esphome.codegen as cg
from esphome.components import i2c, switch
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    STATE_CLASS_MEASUREMENT,
    DEVICE_CLASS_SIGNAL_STRENGTH,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_DECIBEL_MILLIWATT,
    DEVICE_CLASS_DURATION,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_TIMER,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_SECOND,
    ICON_BLUETOOTH,
    DEVICE_CLASS_SWITCH,
    ENTITY_CATEGORY_CONFIG,
)

DEPENDENCIES = ["i2c"] # client depends on i2c master (extends I2CDevice)

# CONF_WIFI_SIGNAL = "wifi_signal"
# CONF_UPTIME = "uptime"
CONF_I2C_REG_KEY = "i2c_registry_key"
CONF_SWITCH = "switch"
ICON_TOGGLE = "mdi:toggle-switch"

i2c_client_ns = cg.esphome_ns.namespace("i2c_client")
I2CClientSwitch = i2c_client_ns.class_("I2CClientSwitch", switch.Switch, cg.Component, i2c.I2CDevice)

# CONFIG_SCHEMA = (
#     switch.switch_schema(BLEClientSwitch, icon=ICON_BLUETOOTH, block_inverted=True)
#     .extend(ble_client.BLE_CLIENT_SCHEMA)
#     .extend(cv.COMPONENT_SCHEMA)
# )

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(I2CClientSwitch),
            cv.Required(CONF_I2C_REG_KEY): cv.hex_uint8_t,
            cv.Optional(CONF_SWITCH): switch.switch_schema(
                I2CClientSwitch,
                device_class=DEVICE_CLASS_SWITCH,
                block_inverted=True,
                icon=ICON_TOGGLE,
                # entity_category=ENTITY_CATEGORY_CONFIG,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x0))
)

TYPES = {
    CONF_SWITCH: "set_switch",
}

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_registry_key(config[CONF_I2C_REG_KEY]))

    # sw = await switch.new_switch(config[CONF_SWITCH])
    # cg.add(sw.set_switch(config[CONF_SWITCH]))

    for key, funcName in TYPES.items():
        if key in config:
            sw = await switch.new_switch(config[key])
            # await cg.register_parented(sw, var)
            cg.add(getattr(var, funcName)(sw))

    # if sw_config := config.get(CONF_SWITCH):
    #     # s = await switch.new_switch(sw_config, var)
    #     s = await switch.new_switch(sw_config)
    #     # await cg.register_parented(s, config[CONF_ID])
    #     cg.add(I2CClientSwitch.set_switch(s))

    # if stove_config := config.get(CONF_STOVE):
    #     sw = await switch.new_switch(stove_config, mv)
    #     cg.add(mv.set_stove(sw))