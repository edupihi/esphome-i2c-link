import esphome.codegen as cg
from esphome.components import i2c_slave, i2c_service, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_HUMIDITY,
    CONF_ID,
    CONF_TEMPERATURE,
    CONF_VARIANT,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
)

DEPENDENCIES = ["i2c_slave","sensor"] # sensor service depends on i2c slave and sensor (extends I2CSlaveDevice)

CONF_I2C_SLAVE_ID = "i2c_slave_id"
CONF_I2C_REG_KEY = "i2c_registry_key"
CONF_I2C_SVC_SENSOR_ID = "i2c_svc_sensor_id"

i2c_service_ns = cg.esphome_ns.namespace("i2c_service")
I2CServiceComponent = i2c_service_ns.class_("I2CServiceComponent", cg.PollingComponent, i2c_slave.I2CSlaveDevice)

def i2c_service_sensor_schema():
    """Create a schema for a sensor to be registered as i2c slave.

    :return: The i2c service sensor schema, `extend` this in your config schema.
    """
    schema = {
        cv.GenerateID(CONF_I2C_SVC_SENSOR_ID): cv.use_id(sensor.Sensor),
    }
    return cv.Schema(schema)

async def register_i2c_service_sensor(var, config):
    """Register a sensor to this service.

    This is a coroutine, you need to await it with a 'yield' expression!
    """
    parent = await cg.get_variable(config[CONF_I2C_SVC_SENSOR_ID])
    cg.add(var.set_sensor(parent))

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(I2CServiceComponent),
            cv.Required(CONF_I2C_REG_KEY): cv.hex_uint16_t,
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c_slave.i2c_slave_device_schema())
    .extend(i2c_service_sensor_schema())
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await i2c_slave.register_i2c_slave_device(var, config)
    await register_i2c_service_sensor(var, config)

    cg.add(var.set_registry_key(config[CONF_I2C_REG_KEY]))

