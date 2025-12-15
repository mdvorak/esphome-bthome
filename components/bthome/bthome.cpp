#include "bthome.h"
#include "esphome/core/log.h"

#if defined(USE_ESP32) || defined(USE_NRF52)

#include <cstring>
#include <cmath>

// Platform-specific includes
#ifdef USE_ESP32
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include "esphome/core/hal.h"
#include "mbedtls/ccm.h"
#endif

#ifdef USE_NRF52
#include <zephyr/kernel.h>
#include <tinycrypt/ccm_mode.h>
#include <tinycrypt/constants.h>
#endif

namespace esphome {
namespace bthome {

static const char *const TAG = "bthome";

void BTHome::dump_config() {
  ESP_LOGCONFIG(TAG,
                "BTHome:\n"
                "  Min Interval: %ums\n"
                "  Max Interval: %ums\n"
                "  TX Power: %ddBm\n"
                "  Encryption: %s\n"
                "  Sensors: %d\n"
                "  Binary Sensors: %d",
                this->min_interval_, this->max_interval_,
#ifdef USE_ESP32
                (this->tx_power_esp32_ * 3) - 12,
#else
                this->tx_power_nrf52_,
#endif
                this->encryption_enabled_ ? "enabled" : "disabled",
                this->measurements_.size(), this->binary_measurements_.size());
}

float BTHome::get_setup_priority() const {
#ifdef USE_ESP32
  return setup_priority::AFTER_BLUETOOTH;
#else
  return setup_priority::BLUETOOTH;
#endif
}

void BTHome::setup() {
  ESP_LOGD(TAG, "Setting up BTHome...");

#ifdef USE_ESP32
  // ESP32: Set up BLE advertising parameters
  this->ble_adv_params_ = {
      .adv_int_min = static_cast<uint16_t>(this->min_interval_ / 0.625f),
      .adv_int_max = static_cast<uint16_t>(this->max_interval_ / 0.625f),
      .adv_type = ADV_TYPE_NONCONN_IND,
      .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
      .peer_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      .peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
      .channel_map = ADV_CHNL_ALL,
      .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
  };

  global_ble->advertising_register_raw_advertisement_callback([this](bool advertise) {
    this->advertising_ = advertise;
    if (advertise) {
      this->build_advertisement_data_();
      this->start_advertising_();
    }
  });
#endif

#ifdef USE_NRF52
  // nRF52: Initialize Bluetooth
  int err = bt_enable(nullptr);
  if (err) {
    ESP_LOGE(TAG, "Bluetooth init failed (err %d)", err);
    this->mark_failed();
    return;
  }

  ESP_LOGD(TAG, "Bluetooth initialized");

  // Set up advertising parameters
  this->adv_param_ = BT_LE_ADV_PARAM_INIT(
      BT_LE_ADV_OPT_USE_IDENTITY,
      BT_GAP_ADV_FAST_INT_MIN_2,
      BT_GAP_ADV_FAST_INT_MAX_2,
      nullptr
  );
  this->adv_param_.interval_min = this->min_interval_ * 1000 / 625;
  this->adv_param_.interval_max = this->max_interval_ * 1000 / 625;
#endif

  // Register callbacks for sensor state changes
  for (size_t i = 0; i < this->measurements_.size(); i++) {
    auto &measurement = this->measurements_[i];
    measurement.sensor->add_on_state_callback([this, i](float) {
      if (this->measurements_[i].advertise_immediately) {
        this->trigger_immediate_advertising_(i, false);
      } else {
        this->data_changed_ = true;
      }
    });
  }

  for (size_t i = 0; i < this->binary_measurements_.size(); i++) {
    auto &measurement = this->binary_measurements_[i];
    measurement.sensor->add_on_state_callback([this, i](bool) {
      if (this->binary_measurements_[i].advertise_immediately) {
        this->trigger_immediate_advertising_(i, true);
      } else {
        this->data_changed_ = true;
      }
    });
  }

#ifdef USE_NRF52
  // nRF52: Build and start advertising immediately
  this->build_advertisement_data_();
  this->start_advertising_();
#endif

#ifdef USE_ESP32
  // ESP32: Disable loop initially - only enable for immediate advertising
  this->disable_loop();
#endif
}

void BTHome::loop() {
  // Handle immediate advertising requests
  if (this->immediate_advertising_pending_) {
    this->immediate_advertising_pending_ = false;
    this->stop_advertising_();
    this->build_advertisement_data_();
    this->start_advertising_();
#ifdef USE_ESP32
    this->disable_loop();
#endif
    return;
  }

  // Handle regular data changes
  if (this->data_changed_ && this->advertising_) {
    this->data_changed_ = false;
    this->stop_advertising_();
    this->build_advertisement_data_();
    this->start_advertising_();
  }
}

void BTHome::set_encryption_key(const std::array<uint8_t, 16> &key) {
  this->encryption_enabled_ = true;
  this->encryption_key_ = key;
}

void BTHome::add_measurement(sensor::Sensor *sensor, uint8_t object_id, uint8_t data_bytes,
                              bool is_signed, float factor, bool advertise_immediately) {
  this->measurements_.push_back({sensor, object_id, data_bytes, is_signed, factor, advertise_immediately});
}

void BTHome::add_binary_measurement(binary_sensor::BinarySensor *sensor, uint8_t object_id, bool advertise_immediately) {
  this->binary_measurements_.push_back({sensor, object_id, advertise_immediately});
}

void BTHome::trigger_immediate_advertising_(uint8_t measurement_index, bool is_binary) {
  this->immediate_advertising_pending_ = true;
  this->immediate_adv_measurement_index_ = measurement_index;
  this->immediate_adv_is_binary_ = is_binary;
#ifdef USE_ESP32
  this->enable_loop();
#endif
}

void BTHome::build_advertisement_data_() {
  size_t pos = 0;

  // Flags AD element
  this->adv_data_[pos++] = 0x02;  // Length
  this->adv_data_[pos++] = 0x01;  // Type: Flags
  this->adv_data_[pos++] = 0x06;  // LE General Discoverable, BR/EDR not supported

  // Service Data AD element
  size_t service_data_len_pos = pos;
  pos++;  // Length placeholder
  this->adv_data_[pos++] = 0x16;  // Type: Service Data

  // BTHome Service UUID (little-endian)
  this->adv_data_[pos++] = BTHOME_SERVICE_UUID & 0xFF;
  this->adv_data_[pos++] = (BTHOME_SERVICE_UUID >> 8) & 0xFF;

  // Device info byte
  uint8_t device_info = this->encryption_enabled_ ? BTHOME_DEVICE_INFO_ENCRYPTED : BTHOME_DEVICE_INFO_UNENCRYPTED;
  this->adv_data_[pos++] = device_info;

  size_t measurement_start = pos;

  // Handle immediate advertising - single sensor only
  if (this->immediate_advertising_pending_) {
    if (this->immediate_adv_is_binary_) {
      auto &measurement = this->binary_measurements_[this->immediate_adv_measurement_index_];
      if (measurement.sensor->has_state()) {
        pos += this->encode_binary_measurement_(this->adv_data_ + pos, MAX_BLE_ADVERTISEMENT_SIZE - pos,
                                                 measurement.object_id, measurement.sensor->state);
      }
    } else {
      auto &measurement = this->measurements_[this->immediate_adv_measurement_index_];
      if (measurement.sensor->has_state() && !std::isnan(measurement.sensor->state)) {
        pos += this->encode_measurement_(this->adv_data_ + pos, MAX_BLE_ADVERTISEMENT_SIZE - pos, measurement);
      }
    }
  } else {
    // Normal: add all measurements that fit
    for (const auto &measurement : this->measurements_) {
      if (!measurement.sensor->has_state() || std::isnan(measurement.sensor->state))
        continue;

      // Check if measurement fits: object_id (1 byte) + data_bytes
      size_t encoded_size = 1 + measurement.data_bytes;
      if (pos + encoded_size > MAX_BLE_ADVERTISEMENT_SIZE)
        break;

      pos += this->encode_measurement_(this->adv_data_ + pos, MAX_BLE_ADVERTISEMENT_SIZE - pos, measurement);
    }

    for (const auto &measurement : this->binary_measurements_) {
      if (!measurement.sensor->has_state())
        continue;

      if (pos + 2 > MAX_BLE_ADVERTISEMENT_SIZE)
        break;

      pos += this->encode_binary_measurement_(this->adv_data_ + pos, MAX_BLE_ADVERTISEMENT_SIZE - pos,
                                               measurement.object_id, measurement.sensor->state);
    }
  }

  size_t measurement_len = pos - measurement_start;

  // Handle encryption
  if (this->encryption_enabled_ && measurement_len > 0) {
    uint8_t plaintext[MAX_BLE_ADVERTISEMENT_SIZE];
    memcpy(plaintext, this->adv_data_ + measurement_start, measurement_len);

    uint8_t ciphertext[MAX_BLE_ADVERTISEMENT_SIZE];
    size_t ciphertext_len = 0;

    if (this->encrypt_payload_(plaintext, measurement_len, ciphertext, &ciphertext_len)) {
      memcpy(this->adv_data_ + measurement_start, ciphertext, ciphertext_len);
      pos = measurement_start + ciphertext_len;

      // Add counter (4 bytes, little-endian)
      this->adv_data_[pos++] = this->counter_ & 0xFF;
      this->adv_data_[pos++] = (this->counter_ >> 8) & 0xFF;
      this->adv_data_[pos++] = (this->counter_ >> 16) & 0xFF;
      this->adv_data_[pos++] = (this->counter_ >> 24) & 0xFF;

      this->counter_++;
    }
  }

  // Set service data length
  this->adv_data_[service_data_len_pos] = pos - service_data_len_pos - 1;

  this->adv_data_len_ = pos;

  ESP_LOGD(TAG, "Built advertisement data (%d bytes)", this->adv_data_len_);
}

void BTHome::start_advertising_() {
#ifdef USE_ESP32
  ESP_LOGD(TAG, "Setting BLE TX power");
  esp_err_t err = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, this->tx_power_esp32_);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "esp_ble_tx_power_set failed: %s", esp_err_to_name(err));
  }

  ESP_LOGD(TAG, "Starting BTHome advertisement (%d bytes)", this->adv_data_len_);
  err = esp_ble_gap_config_adv_data_raw(this->adv_data_, this->adv_data_len_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_ble_gap_config_adv_data_raw failed: %s", esp_err_to_name(err));
  }
#endif

#ifdef USE_NRF52
  static uint8_t flags_data[] = {BT_LE_AD_NO_BREDR | BT_LE_AD_GENERAL};

  this->ad_[0].type = BT_DATA_FLAGS;
  this->ad_[0].data_len = sizeof(flags_data);
  this->ad_[0].data = flags_data;

  // Service data (skip flags we already added)
  this->ad_[1].type = BT_DATA_SVC_DATA16;
  this->ad_[1].data_len = this->adv_data_len_ - 3;  // Skip flags
  this->ad_[1].data = this->adv_data_ + 4;          // Skip flags + length + type

  int err = bt_le_adv_start(&this->adv_param_, this->ad_, 2, nullptr, 0);
  if (err) {
    ESP_LOGE(TAG, "Advertising failed to start (err %d)", err);
    return;
  }

  this->advertising_ = true;
  ESP_LOGD(TAG, "BTHome advertising started");
#endif
}

void BTHome::stop_advertising_() {
#ifdef USE_ESP32
  if (this->advertising_) {
    esp_ble_gap_stop_advertising();
  }
#endif

#ifdef USE_NRF52
  if (this->advertising_) {
    bt_le_adv_stop();
    this->advertising_ = false;
  }
#endif
}

#ifdef USE_ESP32
void BTHome::gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
  if (!this->advertising_)
    return;

  esp_err_t err;
  switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
      err = esp_ble_gap_start_advertising(&this->ble_adv_params_);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ble_gap_start_advertising failed: %s", esp_err_to_name(err));
      }
      break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
      if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "BLE adv start failed: %d", param->adv_start_cmpl.status);
      } else {
        ESP_LOGD(TAG, "BLE advertising started");
      }
      break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
      if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "BLE adv stop failed: %d", param->adv_stop_cmpl.status);
      }
      break;
    default:
      break;
  }
}
#endif

size_t BTHome::encode_measurement_(uint8_t *data, size_t max_len, const SensorMeasurement &measurement) {
  // Generic BTHome v2 sensor encoding
  // Uses data_bytes, is_signed, and factor from the measurement struct
  // See: https://bthome.io/format/

  size_t required_size = 1 + measurement.data_bytes;  // object_id + value bytes
  if (max_len < required_size) {
    return 0;
  }

  float value = measurement.sensor->state;
  size_t pos = 0;

  // Object ID
  data[pos++] = measurement.object_id;

  // Convert value to encoded integer using factor
  // Factor in Python is the resolution (e.g., 0.01 means value * 100)
  // So we divide by factor to get the encoded value
  double scaled = std::round(value / measurement.factor);

  // Encode based on data_bytes and signedness (little-endian)
  if (measurement.is_signed) {
    // Signed integer encoding
    int32_t encoded;
    switch (measurement.data_bytes) {
      case 1:
        encoded = static_cast<int8_t>(std::max(-128.0, std::min(127.0, scaled)));
        data[pos++] = encoded & 0xFF;
        break;
      case 2:
        encoded = static_cast<int16_t>(std::max(-32768.0, std::min(32767.0, scaled)));
        data[pos++] = encoded & 0xFF;
        data[pos++] = (encoded >> 8) & 0xFF;
        break;
      case 3:
        // sint24 - range: -8388608 to 8388607
        encoded = static_cast<int32_t>(std::max(-8388608.0, std::min(8388607.0, scaled)));
        data[pos++] = encoded & 0xFF;
        data[pos++] = (encoded >> 8) & 0xFF;
        data[pos++] = (encoded >> 16) & 0xFF;
        break;
      case 4:
        encoded = static_cast<int32_t>(scaled);
        data[pos++] = encoded & 0xFF;
        data[pos++] = (encoded >> 8) & 0xFF;
        data[pos++] = (encoded >> 16) & 0xFF;
        data[pos++] = (encoded >> 24) & 0xFF;
        break;
      default:
        ESP_LOGW(TAG, "Unsupported data_bytes: %d for object 0x%02X", measurement.data_bytes, measurement.object_id);
        return 0;
    }
  } else {
    // Unsigned integer encoding
    uint32_t encoded;
    switch (measurement.data_bytes) {
      case 1:
        encoded = static_cast<uint8_t>(std::max(0.0, std::min(255.0, scaled)));
        data[pos++] = encoded & 0xFF;
        break;
      case 2:
        encoded = static_cast<uint16_t>(std::max(0.0, std::min(65535.0, scaled)));
        data[pos++] = encoded & 0xFF;
        data[pos++] = (encoded >> 8) & 0xFF;
        break;
      case 3:
        // uint24 - range: 0 to 16777215
        encoded = static_cast<uint32_t>(std::max(0.0, std::min(16777215.0, scaled)));
        data[pos++] = encoded & 0xFF;
        data[pos++] = (encoded >> 8) & 0xFF;
        data[pos++] = (encoded >> 16) & 0xFF;
        break;
      case 4:
        encoded = static_cast<uint32_t>(std::max(0.0, scaled));
        data[pos++] = encoded & 0xFF;
        data[pos++] = (encoded >> 8) & 0xFF;
        data[pos++] = (encoded >> 16) & 0xFF;
        data[pos++] = (encoded >> 24) & 0xFF;
        break;
      default:
        ESP_LOGW(TAG, "Unsupported data_bytes: %d for object 0x%02X", measurement.data_bytes, measurement.object_id);
        return 0;
    }
  }

  return pos;
}

size_t BTHome::encode_binary_measurement_(uint8_t *data, size_t max_len, uint8_t object_id, bool value) {
  // Binary sensors are always encoded as: [object_id] [0x00 or 0x01]
  // See: https://bthome.io/format/
  //
  // BTHome v2 binary sensor object IDs:
  //   0x0F - generic_boolean (generic on/off)
  //   0x10 - power (power on/off)
  //   0x11 - opening (open/closed)
  //   0x15 - battery_low (battery normal/low)
  //   0x16 - battery_charging (not charging/charging)
  //   0x17 - carbon_monoxide (CO not detected/detected)
  //   0x18 - cold (normal/cold)
  //   0x19 - connectivity (disconnected/connected)
  //   0x1A - door (closed/open)
  //   0x1B - garage_door (closed/open)
  //   0x1C - gas (clear/detected)
  //   0x1D - heat (normal/hot)
  //   0x1E - light (no light/light detected)
  //   0x1F - lock (locked/unlocked)
  //   0x20 - moisture_binary (dry/wet)
  //   0x21 - motion (clear/detected)
  //   0x22 - moving (not moving/moving)
  //   0x23 - occupancy (clear/detected)
  //   0x24 - plug (unplugged/plugged in)
  //   0x25 - presence (away/home)
  //   0x26 - problem (ok/problem)
  //   0x27 - running (not running/running)
  //   0x28 - safety (unsafe/safe)
  //   0x29 - smoke (clear/detected)
  //   0x2A - sound (clear/detected)
  //   0x2B - tamper (off/on)
  //   0x2C - vibration (clear/detected)
  //   0x2D - window (closed/open)

  if (max_len < 2) return 0;

  data[0] = object_id;
  data[1] = value ? 0x01 : 0x00;
  return 2;
}

bool BTHome::encrypt_payload_(const uint8_t *plaintext, size_t plaintext_len, uint8_t *ciphertext, size_t *ciphertext_len) {
  if (!this->encryption_enabled_) return false;

  // Build nonce: MAC (6) + UUID (2) + device info (1) + counter (4) = 13 bytes
  uint8_t nonce[13];

#ifdef USE_ESP32
  const uint8_t *mac = esp_bt_dev_get_address();
  memcpy(nonce, mac, 6);
#endif

#ifdef USE_NRF52
  bt_addr_le_t addr;
  size_t count = 1;
  bt_id_get(&addr, &count);
  memcpy(nonce, addr.a.val, 6);
#endif

  nonce[6] = BTHOME_SERVICE_UUID & 0xFF;
  nonce[7] = (BTHOME_SERVICE_UUID >> 8) & 0xFF;
  nonce[8] = BTHOME_DEVICE_INFO_ENCRYPTED;
  nonce[9] = this->counter_ & 0xFF;
  nonce[10] = (this->counter_ >> 8) & 0xFF;
  nonce[11] = (this->counter_ >> 16) & 0xFF;
  nonce[12] = (this->counter_ >> 24) & 0xFF;

#ifdef USE_ESP32
  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);

  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, this->encryption_key_.data(), 128);
  if (ret != 0) {
    ESP_LOGE(TAG, "mbedtls_ccm_setkey failed: %d", ret);
    mbedtls_ccm_free(&ctx);
    return false;
  }

  ret = mbedtls_ccm_encrypt_and_tag(&ctx, plaintext_len, nonce, sizeof(nonce), nullptr, 0,
                                     plaintext, ciphertext, ciphertext + plaintext_len, 4);
  mbedtls_ccm_free(&ctx);

  if (ret != 0) {
    ESP_LOGE(TAG, "mbedtls_ccm_encrypt_and_tag failed: %d", ret);
    return false;
  }
#endif

#ifdef USE_NRF52
  struct tc_ccm_mode_struct ctx;
  struct tc_aes_key_sched_struct sched;

  if (tc_aes128_set_encrypt_key(&sched, this->encryption_key_.data()) != TC_CRYPTO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to set AES key");
    return false;
  }

  if (tc_ccm_config(&ctx, &sched, nonce, sizeof(nonce), 4) != TC_CRYPTO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure CCM");
    return false;
  }

  if (tc_ccm_generation_encryption(ciphertext, plaintext_len + 4, nullptr, 0,
                                    plaintext, plaintext_len, &ctx) != TC_CRYPTO_SUCCESS) {
    ESP_LOGE(TAG, "CCM encryption failed");
    return false;
  }
#endif

  *ciphertext_len = plaintext_len + 4;
  return true;
}

}  // namespace bthome
}  // namespace esphome

#endif  // USE_ESP32 || USE_NRF52
