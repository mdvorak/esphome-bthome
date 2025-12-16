---
title: Sensor Types
description: Complete reference of all supported BTHome sensor types
---

This page lists all sensor types supported by the BTHome component. Each type corresponds to a BTHome v2 object ID.

## Basic Sensors

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `packet_id` | 0x00 | 1 | - | Packet identifier for deduplication |
| `battery` | 0x01 | 1 | % | Battery level percentage |
| `temperature` | 0x02 | 0.01 | °C | Temperature (signed) |
| `humidity` | 0x03 | 0.01 | % | Relative humidity |
| `pressure` | 0x04 | 0.01 | hPa | Atmospheric pressure |
| `illuminance` | 0x05 | 0.01 | lux | Light intensity |

## Mass Sensors

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `mass_kg` | 0x06 | 0.01 | kg | Mass in kilograms |
| `mass_lb` | 0x07 | 0.01 | lb | Mass in pounds |

## Environmental Sensors

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `dewpoint` | 0x08 | 0.01 | °C | Dew point temperature (signed) |
| `co2` | 0x12 | 1 | ppm | Carbon dioxide level |
| `tvoc` | 0x13 | 1 | µg/m³ | Total volatile organic compounds |
| `pm2_5` | 0x0D | 1 | µg/m³ | PM2.5 particulate matter |
| `pm10` | 0x0E | 1 | µg/m³ | PM10 particulate matter |
| `moisture` | 0x14 | 0.01 | % | Soil/material moisture |

## Electrical Sensors

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `voltage` | 0x0C | 0.001 | V | Voltage |
| `voltage_01` | 0x4A | 0.1 | V | Voltage (lower precision) |
| `current` | 0x43 | 0.001 | A | Current |
| `current_sint16` | 0x5D | 0.001 | A | Current (signed, bidirectional) |
| `power` | 0x0B | 0.01 | W | Power consumption |
| `power_sint32` | 0x5C | 0.01 | W | Power (signed, bidirectional) |
| `energy` | 0x0A | 0.001 | kWh | Energy consumption |
| `energy_uint32` | 0x4D | 0.001 | kWh | Energy (extended range) |
| `conductivity` | 0x56 | 1 | µS/cm | Electrical conductivity |

## Count Sensors

| Type | Object ID | Resolution | Range | Description |
|------|-----------|------------|-------|-------------|
| `count_uint8` | 0x09 | 1 | 0-255 | 8-bit counter |
| `count_uint16` | 0x3D | 1 | 0-65535 | 16-bit counter |
| `count_uint32` | 0x3E | 1 | 0-4B | 32-bit counter |
| `count_sint8` | 0x59 | 1 | ±127 | 8-bit signed counter |
| `count_sint16` | 0x5A | 1 | ±32767 | 16-bit signed counter |
| `count_sint32` | 0x5B | 1 | ±2B | 32-bit signed counter |

## Distance & Motion Sensors

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `distance_mm` | 0x40 | 1 | mm | Distance in millimeters |
| `distance_m` | 0x41 | 0.1 | m | Distance in meters |
| `rotation` | 0x3F | 0.1 | ° | Rotation angle (signed) |
| `speed` | 0x44 | 0.01 | m/s | Speed |
| `acceleration` | 0x51 | 0.001 | m/s² | Acceleration |
| `gyroscope` | 0x52 | 0.001 | °/s | Angular velocity |
| `direction` | 0x5E | 0.01 | ° | Compass direction |

## Volume & Flow Sensors

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `volume_l_01` | 0x47 | 0.1 | L | Volume (low precision) |
| `volume_ml` | 0x48 | 1 | mL | Volume in milliliters |
| `volume_l` | 0x4E | 0.001 | L | Volume in liters |
| `volume_storage` | 0x55 | 0.001 | L | Storage volume |
| `volume_flow_rate` | 0x49 | 0.001 | m³/hr | Flow rate |
| `water` | 0x4F | 0.001 | L | Water consumption |
| `gas` | 0x4B | 0.001 | m³ | Gas consumption (24-bit) |
| `gas_uint32` | 0x4C | 0.001 | m³ | Gas consumption (32-bit) |

## Time Sensors

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `duration` | 0x42 | 0.001 | s | Time duration |
| `timestamp` | 0x50 | 1 | s | Unix timestamp |

## Other Sensors

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `uv_index` | 0x46 | 0.1 | - | UV index |
| `precipitation` | 0x5F | 0.1 | mm | Rainfall |
| `channel` | 0x60 | 1 | - | Channel number |
| `rotational_speed` | 0x61 | 1 | rpm | Rotational speed |

## Alternate Precision Temperature

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `temperature_01` | 0x45 | 0.1 | °C | Temperature (lower precision) |
| `temperature_sint8` | 0x57 | 1 | °C | Temperature (8-bit, integer only) |
| `temperature_sint8_035` | 0x58 | 0.35 | °C | Temperature (8-bit, 0.35°C steps) |

## Alternate Precision Humidity/Moisture

| Type | Object ID | Resolution | Unit | Description |
|------|-----------|------------|------|-------------|
| `humidity_uint8` | 0x2E | 1 | % | Humidity (8-bit, integer only) |
| `moisture_uint8` | 0x2F | 1 | % | Moisture (8-bit, integer only) |

## Usage Example

```yaml
bthome:
  sensors:
    - type: temperature    # Standard 0.01°C resolution
      id: my_temperature
    - type: humidity       # Standard 0.01% resolution
      id: my_humidity
    - type: battery        # 1% resolution
      id: battery_level
    - type: co2           # 1 ppm resolution
      id: co2_sensor
    - type: voltage       # 0.001V resolution
      id: mains_voltage
```

## Choosing the Right Type

- **Temperature**: Use `temperature` (0.01°C) for most cases. Use `temperature_01` to save bandwidth.
- **Humidity**: Use `humidity` (0.01%) for most cases. Use `humidity_uint8` to save bandwidth.
- **Counters**: Choose based on your expected range and whether values can be negative.
- **Power/Current**: Use signed variants (`power_sint32`, `current_sint16`) for bidirectional measurements (solar, batteries).

## See Also

- [Binary Sensor Types](/reference/binary-sensor-types) - Binary sensor reference
- [Sensors Configuration](/configuration/sensors) - How to configure sensors
