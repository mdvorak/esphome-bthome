---
title: nRF52 (XIAO BLE)
description: Using BTHome with nRF52840 boards like Seeed XIAO BLE
---

The BTHome component supports nRF52840 boards using the Zephyr RTOS framework. This is ideal for ultra-low-power battery-operated sensors.

## Supported Boards

- **Seeed XIAO BLE** (nRF52840) - Primary supported board
- Other nRF52840 boards (may require board definition adjustments)

## Basic Configuration

```yaml
esphome:
  name: bthome-nrf52
  friendly_name: BTHome nRF52

nrf52:
  board: xiao_ble
  bootloader: adafruit

logger:

external_components:
  - source:
      type: git
      url: https://github.com/dz0ny/esphome-bthome
      ref: main
    components: [bthome]
```

## Pin Naming

nRF52 uses numeric pin references, not GPIO names:

| Silkscreen | Pin Number | Function |
|------------|------------|----------|
| D0 | 0 | Digital I/O |
| D1 | 1 | Digital I/O |
| D2 | 2 | Digital I/O |
| D3 | 3 | Digital I/O |
| D4 | 4 | I2C SDA |
| D5 | 5 | I2C SCL |
| D6-D10 | 6-10 | Digital I/O |

```yaml
# Correct for nRF52
i2c:
  sda: 4  # D4
  scl: 5  # D5

# NOT like ESP32
# sda: GPIO4  # Wrong!
```

## TX Power Levels

nRF52 supports a wide range of transmit power levels:

| Setting | Description |
|---------|-------------|
| -40 | Ultra low power |
| -20 | Very low power |
| -16 | Low power |
| -12 | Low power |
| -8 | Below default |
| -4 | Below default |
| 0 | Default |
| 2, 3, 4 | Above default |
| 5, 6, 7, 8 | High power |

```yaml
bthome:
  tx_power: 4  # Good balance of range and power
```

## Complete Example

Door sensor with battery monitoring:

```yaml
esphome:
  name: door-sensor
  friendly_name: Door Sensor

nrf52:
  board: xiao_ble
  bootloader: adafruit

logger:

external_components:
  - source:
      type: git
      url: https://github.com/dz0ny/esphome-bthome
      ref: main
    components: [bthome]

# Battery monitoring
sensor:
  - platform: adc
    pin: 29  # VBAT/2 on XIAO BLE
    id: battery_voltage
    name: "Battery Voltage"
    update_interval: 60s
    filters:
      - multiply: 2.0

  - platform: template
    id: battery_percent
    name: "Battery"
    unit_of_measurement: "%"
    lambda: |-
      float voltage = id(battery_voltage).state;
      float percent = (voltage - 3.0) / (4.2 - 3.0) * 100.0;
      if (percent > 100) percent = 100;
      if (percent < 0) percent = 0;
      return percent;
    update_interval: 60s

# Door contact
binary_sensor:
  - platform: gpio
    pin:
      number: 3
      mode: INPUT_PULLUP
    id: door_contact
    name: "Door"
    device_class: door

# BTHome configuration
bthome:
  min_interval: 5s
  max_interval: 10s
  tx_power: 4
  sensors:
    - type: battery
      id: battery_percent
  binary_sensors:
    - type: door
      id: door_contact
      advertise_immediately: true
```

## Deep Sleep

For maximum battery life, enable deep sleep:

```yaml
deep_sleep:
  run_duration: 15s
  sleep_duration: 5min
  wakeup_pin:
    number: 3
    mode: INPUT_PULLUP
    inverted: true  # Wake on LOW (door open)
```

With deep sleep:
- Device wakes every 5 minutes to send sensor updates
- Door opening immediately wakes the device
- Battery can last months to years

## Bootloader Options

XIAO BLE supports several bootloaders:

```yaml
nrf52:
  board: xiao_ble
  bootloader: adafruit  # Default, UF2 upload support
```

| Bootloader | Description |
|------------|-------------|
| `adafruit` | Default, supports UF2 drag-and-drop |
| `adafruit_nrf52_sd132` | With SoftDevice S132 |
| `adafruit_nrf52_sd140_v6` | With SoftDevice S140 v6 |

## Flashing

### UF2 Method (Recommended)

1. Double-tap the reset button to enter bootloader mode
2. A USB drive named "XIAO-SENSE" or "NICENANO" will appear
3. Copy the `.uf2` file to the drive
4. Device will automatically reset and run

The firmware is at:
```
.esphome/build/<device-name>/.pioenvs/<device-name>/zephyr.uf2
```

### DFU Method

Use the `.zip` file for over-the-air updates:
```
.esphome/build/<device-name>/.pioenvs/<device-name>/firmware.zip
```

## Power Consumption

The nRF52840 is optimized for ultra-low power:

| Mode | Current |
|------|---------|
| Active (advertising) | ~5-15mA |
| Deep sleep | ~2-5µA |
| BLE TX (0 dBm) | ~5mA |
| BLE TX (8 dBm) | ~15mA |

With a 400mAh battery and 5-minute sleep intervals, expect 6-12 months of battery life.

## Troubleshooting

### Build fails with "tinycrypt not found"

Ensure you're using the latest component version. The component automatically enables tinycrypt for encryption support.

### Pins not working

Use numeric pin references, not GPIO names:
```yaml
# Correct
pin: 4

# Wrong
pin: GPIO4
```

### Device not appearing in bootloader mode

1. Make sure the USB cable supports data (not charge-only)
2. Double-tap reset quickly (within 0.5s)
3. Try a different USB port

## See Also

- [ESP32 Platform](/platforms/esp32) - Alternative platform
- [Encryption](/configuration/encryption) - Secure your sensor data
- [Basic Setup](/configuration/basic-setup) - General configuration options
