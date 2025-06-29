# esphome-i2c-link
ESPHome components for linking ESP32:s over I2C including i2c_slave-implementation (ESP-IDF only) and wrapper-components

# Architecture

Basic I2C architecture, one master, sets i2c-bus (frequency, etc) and multiple slaves, define their own address.

The I2C bus implementation on master is ESPHome standard implementation (i2c), so it could (maybe) work with ```framework: arduino``` but no guarantees.

The I2C slave implementation is not existing in ESPHome (v.6.0 / 2025Jun) and also dependent on the newer v5.4.1 of esp-idf (with platformio: v54.3.20).

NOTE: The ```esphome``` build script does download the correct versions specified in the yaml, but currently the created python-enviroment is missing package ```rich```. You need to locate the correct python virtual env (some info in the build error), activate this venv (eg. source bin/activate.sh) and run ```python -m pip install rich```.

# Master configuration example

```yaml
...
external_components:
  - source: github://edupihi/esphome-i2c-link
    refresh: 60min
    components: [ i2c_client ]

i2c:
  - id: i2c_bus_sensor  # standard i2c implmentation
    sda: ${pin_i2c_sda}
    scl: ${pin_i2c_scl}

sensor:
  - platform: i2c_client   # is basically an I2CDevice, generic single sensor-value request from i2c-slave 
    i2c_id: i2c_bus_sensor
    id: i2cslave
    address: 0x1b              # address of the i2c_slave
    i2c_registry_key: 0x10     # registry to be requested from i2c_slave
    update_interval: 5s
    wifi_signal:               # type of sensor
      name: "WiFi Signal Slave Device"  # extra settings 
  - platform: i2c_client
    i2c_id: i2c_bus_sensor
    id: i2cslave_2
    address: 0x1b
    i2c_registry_key: 0x11
    uptime:
      name: "Uptime Slave Device"
      filters:
        - lambda: return x / 60.0;
      unit_of_measurement: min
    update_interval: 5s
```

# Slave configuration example

```yaml
esp32:
  board: ...
  framework:
    type: esp-idf
    version: 5.4.1
    platform_version: 54.03.20
    sdkconfig_options:
      CONFIG_I2C_ENABLE_SLAVE_DRIVER_VERSION_2: y
      CONFIG_I2C_ISR_IRAM_SAFE: y
#      CONFIG_I2C_ENABLE_DEBUG_LOG: y

external_components:
  - source: github://edupihi/esphome-i2c-link
    refresh: 20min
    components: [ i2c_slave, i2c_service ]

i2c_slave:
  id: i2c_slave_
  sda: ${pin_i2c_sda}
  scl: ${pin_i2c_scl}
  address: 0x1b # only one address possible

sensor:
  - platform: wifi_signal # example sensor
    name: "WiFi Signal 2"
    id: wifi_signal_2
    update_interval: 60s
  - platform: uptime # example sensor 2
    name: Uptime 2
    id: uptime_2
    update_interval: 5s
    # filters:
    #   - lambda: return x / 60.0;    # if state-value is filtered, the filtered value will be sent on the i2c bus
    # unit_of_measurement: min
  - platform: i2c_service              # platform: i2c_service creates a virtual sensor that relays sensor->state to i2c bus as slave
    # name: svc1                       # name not supported yet, works like "internal: true", not showing on web_server f.e.
    i2c_slave_id: i2c_slave_           # ref to i2c_slave: id
    id: i2c_service_sensor_1           # id for i2c_service
    i2c_registry_key: 0x10             # registry that holds the sensor state (float / 4byte)
    i2c_svc_sensor_id: wifi_signal_2   # ref to sensor: id 
  - platform: i2c_service
    # name: svc2
    i2c_slave_id: i2c_slave_
    id: i2c_service_sensor_2
    i2c_registry_key: 0x11
    i2c_svc_sensor_id: uptime_2

```
