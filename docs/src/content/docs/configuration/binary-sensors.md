---
title: Binary Sensors
description: Configure binary sensor measurements for BTHome broadcasting
---

## Binary Sensor Configuration

Binary sensors represent on/off states like motion detected, door open, etc.

```yaml
binary_sensor:
  - platform: gpio
    pin: GPIO4
    id: motion_sensor
    name: "Motion"
    device_class: motion

bthome:
  binary_sensors:
    - type: motion
      id: motion_sensor
      advertise_immediately: true
```

## Common Examples

### Motion Sensor (PIR)

```yaml
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO4
      mode: INPUT_PULLDOWN
    id: pir_motion
    name: "Motion"
    device_class: motion
    filters:
      - delayed_off: 30s  # Keep motion active for 30s after last trigger

bthome:
  binary_sensors:
    - type: motion
      id: pir_motion
      advertise_immediately: true
```

### Door/Window Contact

```yaml
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO5
      mode: INPUT_PULLUP
    id: door_contact
    name: "Front Door"
    device_class: door

bthome:
  binary_sensors:
    - type: door
      id: door_contact
      advertise_immediately: true
```

### Window Contact

```yaml
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO6
      mode: INPUT_PULLUP
    id: window_contact
    name: "Window"
    device_class: window

bthome:
  binary_sensors:
    - type: window
      id: window_contact
      advertise_immediately: true
```

### Smoke Detector

```yaml
binary_sensor:
  - platform: gpio
    pin: GPIO7
    id: smoke_sensor
    name: "Smoke"
    device_class: smoke

bthome:
  binary_sensors:
    - type: smoke
      id: smoke_sensor
      advertise_immediately: true
```

### Water Leak Sensor

```yaml
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO8
      mode: INPUT_PULLUP
    id: water_leak
    name: "Water Leak"
    device_class: moisture

bthome:
  binary_sensors:
    - type: moisture_binary
      id: water_leak
      advertise_immediately: true
```

### Vibration Sensor

```yaml
binary_sensor:
  - platform: gpio
    pin: GPIO9
    id: vibration
    name: "Vibration"
    device_class: vibration
    filters:
      - delayed_off: 5s

bthome:
  binary_sensors:
    - type: vibration
      id: vibration
      advertise_immediately: true
```

## Multiple Binary Sensors

You can broadcast multiple binary sensors from a single device:

```yaml
binary_sensor:
  - platform: gpio
    pin: GPIO4
    id: motion_sensor
    name: "Motion"

  - platform: gpio
    pin: GPIO5
    id: door_sensor
    name: "Door"

  - platform: gpio
    pin: GPIO6
    id: window_sensor
    name: "Window"

bthome:
  binary_sensors:
    - type: motion
      id: motion_sensor
      advertise_immediately: true
    - type: door
      id: door_sensor
      advertise_immediately: true
    - type: window
      id: window_sensor
      advertise_immediately: true
```

## Immediate Advertising

For binary sensors, `advertise_immediately: true` is highly recommended. This ensures state changes are broadcast instantly rather than waiting for the next advertising interval.

```yaml
bthome:
  binary_sensors:
    - type: motion
      id: pir_motion
      advertise_immediately: true  # Instant notification!
```

Without immediate advertising, there could be a delay of several seconds (depending on your `max_interval` setting) before Home Assistant sees the state change.

## See Also

- [Binary Sensor Types Reference](/reference/binary-sensor-types) - Complete list of all supported types
- [Sensors](/configuration/sensors) - Configure analog sensors
