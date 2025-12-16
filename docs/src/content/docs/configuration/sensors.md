---
title: Sensors
description: Configure sensor measurements for BTHome broadcasting
---

## Sensor Configuration

Add sensors to your BTHome configuration to broadcast their values:

```yaml
sensor:
  - platform: bme280_i2c
    temperature:
      id: my_temperature
      name: "Temperature"
    humidity:
      id: my_humidity
      name: "Humidity"

bthome:
  sensors:
    - type: temperature
      id: my_temperature
    - type: humidity
      id: my_humidity
```

## Common Examples

### Temperature & Humidity (BME280)

```yaml
sensor:
  - platform: bme280_i2c
    address: 0x76
    temperature:
      id: bme_temp
      name: "Temperature"
    humidity:
      id: bme_humidity
      name: "Humidity"
    pressure:
      id: bme_pressure
      name: "Pressure"
    update_interval: 30s

bthome:
  sensors:
    - type: temperature
      id: bme_temp
    - type: humidity
      id: bme_humidity
    - type: pressure
      id: bme_pressure
```

### Battery Level

```yaml
sensor:
  - platform: adc
    pin: GPIO34
    id: battery_voltage
    name: "Battery Voltage"
    update_interval: 60s
    filters:
      - multiply: 2.0  # If using voltage divider

  - platform: template
    id: battery_percent
    name: "Battery"
    unit_of_measurement: "%"
    lambda: |-
      float voltage = id(battery_voltage).state;
      float percent = (voltage - 3.0) / (4.2 - 3.0) * 100.0;
      return clamp(percent, 0.0f, 100.0f);
    update_interval: 60s

bthome:
  sensors:
    - type: battery
      id: battery_percent
```

### Light Sensor (BH1750)

```yaml
sensor:
  - platform: bh1750
    name: "Illuminance"
    id: light_level
    address: 0x23
    update_interval: 30s

bthome:
  sensors:
    - type: illuminance
      id: light_level
```

### CO2 Sensor (SCD40)

```yaml
sensor:
  - platform: scd4x
    co2:
      id: co2_level
      name: "CO2"
    temperature:
      id: scd_temp
      name: "Temperature"
    humidity:
      id: scd_humidity
      name: "Humidity"

bthome:
  sensors:
    - type: co2
      id: co2_level
    - type: temperature
      id: scd_temp
    - type: humidity
      id: scd_humidity
```

### Power Monitoring

```yaml
sensor:
  - platform: hlw8012
    voltage:
      id: mains_voltage
      name: "Voltage"
    current:
      id: mains_current
      name: "Current"
    power:
      id: mains_power
      name: "Power"

bthome:
  sensors:
    - type: voltage
      id: mains_voltage
    - type: current
      id: mains_current
    - type: power
      id: mains_power
```

## Immediate Advertising

For sensors that need to report changes quickly:

```yaml
bthome:
  sensors:
    - type: temperature
      id: my_temperature
      advertise_immediately: true  # Broadcast on every change
```

:::note
Immediate advertising increases power consumption. Use sparingly for battery-powered devices.
:::

## See Also

- [Sensor Types Reference](/reference/sensor-types) - Complete list of all supported sensor types
- [Binary Sensors](/configuration/binary-sensors) - Configure binary sensors
