---
title: Binary Sensor Types
description: Complete reference of all supported BTHome binary sensor types
---

This page lists all binary sensor types supported by the BTHome component. Each type represents an on/off state.

## Binary Sensor Reference

All binary sensors use a single byte: `0x00` = off/false, `0x01` = on/true.

| Type | Object ID | Off State | On State | Use Case |
|------|-----------|-----------|----------|----------|
| `generic_boolean` | 0x0F | Off | On | Generic on/off |
| `power` | 0x10 | Off | On | Power state |
| `opening` | 0x11 | Closed | Open | Generic opening |
| `battery_low` | 0x15 | Normal | Low | Battery warning |
| `battery_charging` | 0x16 | Not charging | Charging | Charging state |
| `carbon_monoxide` | 0x17 | Clear | Detected | CO detector |
| `cold` | 0x18 | Normal | Cold | Cold alert |
| `connectivity` | 0x19 | Disconnected | Connected | Connection state |
| `door` | 0x1A | Closed | Open | Door sensor |
| `garage_door` | 0x1B | Closed | Open | Garage door |
| `gas` | 0x1C | Clear | Detected | Gas detector |
| `heat` | 0x1D | Normal | Hot | Heat alert |
| `light` | 0x1E | No light | Light | Light detector |
| `lock` | 0x1F | Locked | Unlocked | Lock state |
| `moisture_binary` | 0x20 | Dry | Wet | Water leak |
| `motion` | 0x21 | Clear | Detected | Motion sensor |
| `moving` | 0x22 | Not moving | Moving | Movement |
| `occupancy` | 0x23 | Clear | Detected | Occupancy |
| `plug` | 0x24 | Unplugged | Plugged in | Plug state |
| `presence` | 0x25 | Away | Home | Presence |
| `problem` | 0x26 | OK | Problem | Problem indicator |
| `running` | 0x27 | Not running | Running | Running state |
| `safety` | 0x28 | Unsafe | Safe | Safety state |
| `smoke` | 0x29 | Clear | Detected | Smoke detector |
| `sound` | 0x2A | Clear | Detected | Sound detector |
| `tamper` | 0x2B | Off | On | Tamper alert |
| `vibration` | 0x2C | Clear | Detected | Vibration |
| `window` | 0x2D | Closed | Open | Window sensor |

## Categories

### Security Sensors

```yaml
bthome:
  binary_sensors:
    - type: motion      # PIR motion detector
      id: pir_sensor
      advertise_immediately: true
    - type: door        # Door contact
      id: door_sensor
      advertise_immediately: true
    - type: window      # Window contact
      id: window_sensor
      advertise_immediately: true
    - type: tamper      # Tamper switch
      id: tamper_sensor
      advertise_immediately: true
```

### Safety Sensors

```yaml
bthome:
  binary_sensors:
    - type: smoke           # Smoke detector
      id: smoke_sensor
      advertise_immediately: true
    - type: carbon_monoxide # CO detector
      id: co_sensor
      advertise_immediately: true
    - type: gas             # Gas leak detector
      id: gas_sensor
      advertise_immediately: true
    - type: moisture_binary # Water leak
      id: water_sensor
      advertise_immediately: true
```

### Environmental Sensors

```yaml
bthome:
  binary_sensors:
    - type: light       # Light/dark detection
      id: light_sensor
    - type: occupancy   # Room occupancy
      id: occupancy_sensor
    - type: presence    # Home/away
      id: presence_sensor
```

### Device State

```yaml
bthome:
  binary_sensors:
    - type: power           # Device power
      id: power_state
    - type: plug            # Plug connected
      id: plug_state
    - type: battery_low     # Low battery warning
      id: battery_low
    - type: battery_charging # Charging indicator
      id: charging
```

## Usage Examples

### Door/Window Sensor

```yaml
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO5
      mode: INPUT_PULLUP
    id: front_door
    name: "Front Door"
    device_class: door

  - platform: gpio
    pin:
      number: GPIO6
      mode: INPUT_PULLUP
    id: back_window
    name: "Back Window"
    device_class: window

bthome:
  binary_sensors:
    - type: door
      id: front_door
      advertise_immediately: true
    - type: window
      id: back_window
      advertise_immediately: true
```

### Motion with Occupancy

```yaml
binary_sensor:
  - platform: gpio
    pin: GPIO4
    id: pir_motion
    name: "Motion"
    device_class: motion
    filters:
      - delayed_off: 30s

  - platform: template
    id: room_occupied
    name: "Room Occupied"
    device_class: occupancy
    lambda: return id(pir_motion).state;
    filters:
      - delayed_off: 5min  # Stay occupied for 5 min after last motion

bthome:
  binary_sensors:
    - type: motion
      id: pir_motion
      advertise_immediately: true
    - type: occupancy
      id: room_occupied
```

### Multi-Sensor Security Device

```yaml
binary_sensor:
  - platform: gpio
    pin: GPIO4
    id: motion
    device_class: motion
    filters:
      - delayed_off: 30s

  - platform: gpio
    pin:
      number: GPIO5
      mode: INPUT_PULLUP
    id: door
    device_class: door

  - platform: gpio
    pin:
      number: GPIO6
      mode: INPUT_PULLUP
    id: tamper
    device_class: tamper

bthome:
  binary_sensors:
    - type: motion
      id: motion
      advertise_immediately: true
    - type: door
      id: door
      advertise_immediately: true
    - type: tamper
      id: tamper
      advertise_immediately: true
```

## Immediate Advertising

For security and safety sensors, always enable `advertise_immediately`:

```yaml
bthome:
  binary_sensors:
    - type: motion
      id: motion_sensor
      advertise_immediately: true  # Don't wait for interval!
```

This ensures state changes are broadcast instantly, rather than waiting for the next advertising interval.

:::tip
Without immediate advertising, a motion trigger might take several seconds to appear in Home Assistant, defeating the purpose of a motion sensor!
:::

## State Mapping

Binary sensors in ESPHome map to BTHome as follows:

| ESPHome State | BTHome Value | Description |
|---------------|--------------|-------------|
| `false` / OFF | 0x00 | Inactive/clear/closed |
| `true` / ON | 0x01 | Active/detected/open |

The meaning depends on the sensor type. For example:
- `door`: OFF = closed, ON = open
- `motion`: OFF = clear, ON = detected
- `battery_low`: OFF = normal, ON = low battery

## See Also

- [Sensor Types](/reference/sensor-types) - Analog sensor reference
- [Binary Sensors Configuration](/configuration/binary-sensors) - How to configure binary sensors
