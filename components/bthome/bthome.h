#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include <array>

// Platform-specific includes
#ifdef USE_ESP32
#include "esphome/components/esp32_ble/ble.h"
#ifndef CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID
#include <esp_bt.h>
#endif
#include <esp_gap_ble_api.h>
#endif  // USE_ESP32

#ifdef USE_NRF52
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#endif  // USE_NRF52

#if defined(USE_ESP32) || defined(USE_NRF52)

namespace esphome {
namespace bthome {

// BTHome v2 constants
static const uint16_t BTHOME_SERVICE_UUID = 0xFCD2;
static const uint8_t BTHOME_DEVICE_INFO_UNENCRYPTED = 0x40;
static const uint8_t BTHOME_DEVICE_INFO_ENCRYPTED = 0x41;
static const size_t MAX_BLE_ADVERTISEMENT_SIZE = 31;

struct SensorMeasurement {
  sensor::Sensor *sensor;
  uint8_t object_id;
  uint8_t data_bytes;      // Number of bytes to encode (1, 2, 3, or 4)
  bool is_signed;          // True for signed integers, false for unsigned
  float factor;            // Multiply raw value by this to get encoded value
  bool advertise_immediately;
};

struct BinarySensorMeasurement {
  binary_sensor::BinarySensor *sensor;
  uint8_t object_id;
  bool advertise_immediately;
};

#ifdef USE_ESP32
using namespace esp32_ble;

class BTHome : public Component, public GAPEventHandler, public Parented<ESP32BLE> {
#else
class BTHome : public Component {
#endif
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  float get_setup_priority() const override;

  void set_min_interval(uint16_t val) { this->min_interval_ = val; }
  void set_max_interval(uint16_t val) { this->max_interval_ = val; }

#ifdef USE_ESP32
  void set_tx_power(esp_power_level_t val) { this->tx_power_esp32_ = val; }
#endif
#ifdef USE_NRF52
  void set_tx_power(int8_t val) { this->tx_power_nrf52_ = val; }
#endif

  void set_encryption_key(const std::array<uint8_t, 16> &key);
  void add_measurement(sensor::Sensor *sensor, uint8_t object_id, uint8_t data_bytes,
                       bool is_signed, float factor, bool advertise_immediately);
  void add_binary_measurement(binary_sensor::BinarySensor *sensor, uint8_t object_id, bool advertise_immediately);

#ifdef USE_ESP32
  void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) override;
#endif

 protected:
  void build_advertisement_data_();
  void start_advertising_();
  void stop_advertising_();
  size_t encode_measurement_(uint8_t *data, size_t max_len, const SensorMeasurement &measurement);
  size_t encode_binary_measurement_(uint8_t *data, size_t max_len, uint8_t object_id, bool value);
  bool encrypt_payload_(const uint8_t *plaintext, size_t plaintext_len, uint8_t *ciphertext, size_t *ciphertext_len);
  void trigger_immediate_advertising_(uint8_t measurement_index, bool is_binary);

  // Measurements storage
  StaticVector<SensorMeasurement, BTHOME_MAX_MEASUREMENTS> measurements_;
  StaticVector<BinarySensorMeasurement, BTHOME_MAX_BINARY_MEASUREMENTS> binary_measurements_;

  // Common settings
  uint16_t min_interval_{1000};
  uint16_t max_interval_{1000};
  bool advertising_{false};

  // Encryption
  bool encryption_enabled_{false};
  std::array<uint8_t, 16> encryption_key_{};
  uint32_t counter_{0};

  // Advertisement data
  uint8_t adv_data_[MAX_BLE_ADVERTISEMENT_SIZE];
  size_t adv_data_len_{0};
  bool data_changed_{true};

  // Immediate advertising
  bool immediate_advertising_pending_{false};
  uint8_t immediate_adv_measurement_index_{0};
  bool immediate_adv_is_binary_{false};

  // Platform-specific members
#ifdef USE_ESP32
  esp_power_level_t tx_power_esp32_{};
  esp_ble_adv_params_t ble_adv_params_;
#endif

#ifdef USE_NRF52
  int8_t tx_power_nrf52_{0};
  struct bt_le_adv_param adv_param_;
  struct bt_data ad_[2];
#endif
};

}  // namespace bthome
}  // namespace esphome

#endif  // USE_ESP32 || USE_NRF52
