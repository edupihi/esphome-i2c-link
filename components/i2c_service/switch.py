import esphome.codegen as cg
from esphome.components import i2c_slave, switch
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    # ENTITY_CATEGORY_NONE,
    # DEVICE_CLASS_SWITCH,
)

DEPENDENCIES = ["i2c_slave","switch"] # switch service depends on i2c slave and switch (extends I2CSlaveDevice)

CONF_I2C_SLAVE_ID = "i2c_slave_id"
CONF_I2C_REG_KEY_TOGGLE = "i2c_registry_key_toggle"
CONF_I2C_REG_KEY_STATE = "i2c_registry_key_state"
CONF_I2C_SVC_SWITCH_ID = "i2c_svc_switch_id"
CONF_SWITCH = "switch"
ICON_TOGGLE = "mdi:toggle-switch"

i2c_service_ns = cg.esphome_ns.namespace("i2c_service")
# I2CServiceSwitchComponent = i2c_service_ns.class_("I2CServiceSwitchComponent", cg.Component, i2c_slave.I2CSlaveDevice)
I2CServiceSwitchComponent = i2c_service_ns.class_("I2CServiceSwitchComponent", cg.PollingComponent, i2c_slave.I2CSlaveDevice)

def i2c_service_switch_schema():
    """Create a schema for a switch to be registered as i2c slave.

    :return: The i2c service switch schema, `extend` this in your config schema.
    """
    schema = {
        cv.GenerateID(CONF_I2C_SVC_SWITCH_ID): cv.use_id(switch.Switch),
    }
    return cv.Schema(schema)

async def register_i2c_service_switch(var, config):
    """Register a switch to this service.

    This is a coroutine, you need to await it with a 'yield' expression!
    """
    parent = await cg.get_variable(config[CONF_I2C_SVC_SWITCH_ID])
    cg.add(var.set_switch(parent))

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(I2CServiceSwitchComponent),
            cv.Required(CONF_I2C_REG_KEY_TOGGLE): cv.hex_uint8_t,
            cv.Required(CONF_I2C_REG_KEY_STATE): cv.hex_uint8_t,
        }
    )
    # .extend(cv.COMPONENT_SCHEMA)
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c_slave.i2c_slave_device_schema())
    .extend(i2c_service_switch_schema())
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await i2c_slave.register_i2c_slave_device(var, config)
    await register_i2c_service_switch(var, config)

    cg.add(var.set_registry_key_toggle(config[CONF_I2C_REG_KEY_TOGGLE]))
    cg.add(var.set_registry_key_state(config[CONF_I2C_REG_KEY_STATE]))

