import esphome.codegen as cg
from esphome.components import i2c, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    # CONF_HUMIDITY,
    # CONF_TEMPERATURE,
    # CONF_VARIANT,
    # DEVICE_CLASS_HUMIDITY,
    # DEVICE_CLASS_TEMPERATURE,
    # UNIT_CELSIUS,
    # UNIT_PERCENT,
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
)

DEPENDENCIES = ["i2c"] # client depends on i2c master (extends I2CDevice)

CONF_WIFI_SIGNAL = "wifi_signal"
CONF_UPTIME = "uptime"
CONF_I2C_REG_KEY = "i2c_registry_key"

i2c_client_ns = cg.esphome_ns.namespace("i2c_client")
I2CClientComponent = i2c_client_ns.class_("I2CClientComponent", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(I2CClientComponent),
            cv.Required(CONF_I2C_REG_KEY): cv.hex_uint8_t,
            cv.Optional(CONF_WIFI_SIGNAL): sensor.sensor_schema(
                unit_of_measurement=UNIT_DECIBEL_MILLIWATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_UPTIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_SECOND,
                icon=ICON_TIMER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_TOTAL_INCREASING,
                device_class=DEVICE_CLASS_DURATION,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c.i2c_device_schema(0x0))
)

TYPES = {
    CONF_WIFI_SIGNAL: "set_sensor",
    CONF_UPTIME: "set_sensor",
    # CONF_HUMIDITY: "set_humidity_sensor",
}


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    # if temperature := config.get(CONF_TEMPERATURE):
    #     sens = await sensor.new_sensor(temperature)
    #     cg.add(var.set_temperature_sensor(sens))

    # if humidity := config.get(CONF_HUMIDITY):
    #     sens = await sensor.new_sensor(humidity)
    #     cg.add(var.set_humidity_sensor(sens))

    cg.add(var.set_registry_key(config[CONF_I2C_REG_KEY]))

    for key, funcName in TYPES.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, funcName)(sens))