---
title: 2-Gang Switch
description: Battery-powered dual switch with BTHome
---

A battery-efficient dual switch using nRF52 that broadcasts both switch states via BTHome.

## Hardware

- Seeed XIAO BLE (nRF52840)
- 2x Toggle switches or rocker switches
- 3.7V LiPo battery

## Wiring

| Component | XIAO Pin |
|-----------|----------|
| Switch 1 | D2 → GND |
| Switch 2 | D3 → GND |
| Battery+ | BAT+ |
| Battery- | BAT- |

## Configuration

```yaml
esphome:
  name: switch-2gang
  friendly_name: 2-Gang Switch

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
    pin: 29
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

# Switches
binary_sensor:
  - platform: gpio
    pin:
      number: 2
      mode: INPUT_PULLUP
    id: switch_1
    name: "Switch 1"
    filters:
      - invert:
      - delayed_on: 10ms
      - delayed_off: 10ms

  - platform: gpio
    pin:
      number: 3
      mode: INPUT_PULLUP
    id: switch_2
    name: "Switch 2"
    filters:
      - invert:
      - delayed_on: 10ms
      - delayed_off: 10ms

# BTHome broadcast
bthome:
  min_interval: 5s
  max_interval: 30s
  tx_power: 4
  sensors:
    - type: battery
      id: battery_percent
  binary_sensors:
    - type: generic_boolean
      id: switch_1
      advertise_immediately: true
    - type: generic_boolean
      id: switch_2
      advertise_immediately: true

# Deep sleep - wake on either switch
deep_sleep:
  run_duration: 15s
  sleep_duration: 5min
  wakeup_pin:
    number: 2
    mode: INPUT_PULLUP
    inverted: true
```

:::note
With this configuration, only switch 1 (D2) can wake the device from deep sleep. For both switches to wake, consider using an OR gate circuit or keeping the device awake longer.
:::

## Power Consumption

| State | Current |
|-------|---------|
| Deep sleep | ~3µA |
| Active | ~5mA |

**Expected battery life**: 6-12 months with 400mAh battery
