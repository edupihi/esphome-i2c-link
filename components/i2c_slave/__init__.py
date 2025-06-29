from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ADDRESS,
    CONF_ID,
    CONF_I2C_ID,
    CONF_INPUT,
    CONF_OUTPUT,
    CONF_SCL,
    CONF_SDA,
    PLATFORM_ESP32,
)
from esphome.core import CORE, coroutine_with_priority

CODEOWNERS = ["@pihiandreas"]

i2c_ns = cg.esphome_ns.namespace("i2c_slave")
I2CSlave = i2c_ns.class_("I2CSlave")
IDFI2CSlave = i2c_ns.class_("IDFI2CSlave", I2CSlave, cg.Component)
I2CSlaveDevice = i2c_ns.class_("I2CSlaveDevice")

CONF_I2C_SLAVE_ID = "i2c_slave_id"

def _slave_declare_type(value):
    if CORE.using_esp_idf:
        return cv.declare_id(IDFI2CSlave)(value)
    raise NotImplementedError

pin_with_input_and_output_support = pins.internal_gpio_pin_number(
    {CONF_OUTPUT: True, CONF_INPUT: True}
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): _slave_declare_type,
            # cv.Required(CONF_ADDRESS): cv.All(cv.hex_int, cv.Range(min=0, max=0x7F)), # max 7 bit addresses
            # cv.Required(CONF_ADDRESS): cv.i2c_address,
            cv.Optional(CONF_SDA, default="SDA"): pin_with_input_and_output_support,
            cv.Optional(CONF_SCL, default="SCL"): pin_with_input_and_output_support,
            cv.Required(CONF_ADDRESS): cv.i2c_address,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_on([PLATFORM_ESP32]),
)

@coroutine_with_priority(1.0)
async def to_code(config):
    cg.add_global(i2c_ns.using)
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_sda_pin(config[CONF_SDA]))
    cg.add(var.set_scl_pin(config[CONF_SCL]))
    cg.add(var.set_i2c_address(config[CONF_ADDRESS]))

def i2c_slave_device_schema():
    """Create a schema for a i2c slave device.

    :return: The i2c slave device schema, `extend` this in your config schema.
    """
    schema = {
        cv.GenerateID(CONF_I2C_SLAVE_ID): cv.use_id(I2CSlave),
        # cv.Required(CONF_ADDRESS): cv.i2c_address,
    }
    return cv.Schema(schema)


async def register_i2c_slave_device(var, config):
    """Register an i2c device with the given config.

    This is a coroutine, you need to await it with a 'yield' expression!
    """
    parent = await cg.get_variable(config[CONF_I2C_SLAVE_ID])
    cg.add(var.set_i2c_slave(parent))
    # cg.add(var.set_i2c_address(config[CONF_ADDRESS]))


def final_validate_device_schema(
    name: str,
    # *,
    # min_timeout: cv.time_period = None,
    # max_timeout: cv.time_period = None,
):
    hub_schema = {}

    # if (min_timeout is not None) and (max_timeout is not None):
    #     hub_schema[cv.Required(CONF_TIMEOUT)] = cv.Range(
    #         min=cv.time_period(min_timeout),
    #         min_included=True,
    #         max=cv.time_period(max_timeout),
    #         max_included=True,
    #         msg=f"Component {name} requires a timeout between {min_timeout} and {max_timeout} for the I2C bus",
    #     )
    # elif min_timeout is not None:
    #     hub_schema[cv.Required(CONF_TIMEOUT)] = cv.Range(
    #         min=cv.time_period(min_timeout),
    #         min_included=True,
    #         msg=f"Component {name} requires a minimum timeout of {min_timeout} for the I2C bus",
    #     )
    # elif max_timeout is not None:
    #     hub_schema[cv.Required(CONF_TIMEOUT)] = cv.Range(
    #         max=cv.time_period(max_timeout),
    #         max_included=True,
    #         msg=f"Component {name} cannot be used with a timeout of over {max_timeout} for the I2C bus",
    #     )

    return cv.Schema(
        {cv.Required(CONF_I2C_ID): fv.id_declaration_match_schema(hub_schema)},
        extra=cv.ALLOW_EXTRA,
    )

