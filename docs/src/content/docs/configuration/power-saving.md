---
title: Power Saving
description: Reduce power consumption for battery-powered BTHome devices
---

BTHome devices are often battery-powered. These configurations help minimize power consumption.

## CPU Frequency (ESP32)

Lower the CPU frequency from 240MHz (default) to 80MHz for significant power savings:

```yaml
esp32:
  board: seeed_xiao_esp32s3
  framework:
    type: esp-idf
    sdkconfig_options:
      CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_80: y
```

Available frequencies:
- `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240` - 240MHz (default, highest performance)
- `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_160` - 160MHz (balanced)
- `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_80` - 80MHz (lowest power)

:::tip[When to use 80MHz]
80MHz is sufficient for most BTHome applications since BLE advertising and sensor reading don't require high CPU speeds.
:::

## WiFi Power Saving

Combine disabling WiFi on boot with power save mode for maximum savings:

```yaml
wifi:
  enable_on_boot: false
  ap: {}
  power_save_mode: HIGH
```

### Options

**`enable_on_boot: false`** - WiFi radio stays off at startup. BLE continues to work independently.

**`power_save_mode`** - Controls WiFi power consumption when enabled:
- `NONE` - No power saving (default)
- `LIGHT` - Light power saving, minimal latency impact
- `HIGH` - Maximum power saving, higher latency

:::caution[OTA Updates]
With `enable_on_boot: false`, OTA updates won't work until WiFi is enabled programmatically. Flash via USB or add a mechanism to enable WiFi when needed.
:::

## Complete Low-Power Example

```yaml
esphome:
  name: bthome-sensor
  friendly_name: BTHome Sensor

esp32:
  board: seeed_xiao_esp32s3
  framework:
    type: esp-idf
    sdkconfig_options:
      CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_80: y

logger:

wifi:
  enable_on_boot: false
  ap: {}
  power_save_mode: HIGH

esp32_ble:
  id: ble

sensor:
  - platform: internal_temperature
    id: temp
    update_interval: 60s  # Less frequent updates

bthome:
  min_interval: 30s  # Longer advertising interval
  max_interval: 60s
  tx_power: -6      # Lower TX power
  sensors:
    - type: temperature
      id: temp
```

## Power Saving Checklist

| Setting | Impact | Recommendation |
|---------|--------|----------------|
| CPU frequency | High | Use 80MHz for BLE-only devices |
| WiFi on boot | High | Disable with `enable_on_boot: false` |
| WiFi power save | Medium | Use `power_save_mode: HIGH` |
| Advertising interval | Medium | 30s+ for battery devices |
| TX power | Medium | Lower if range permits |
| Sensor update interval | Low | Match to advertising interval |
| Logger | Low | Disable in production |

## Disabling Logger

For production deployments, disable logging to save additional power:

```yaml
logger:
  level: NONE
```

Or remove the `logger:` section entirely.
