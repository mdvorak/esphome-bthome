#include "epdiy_epaper.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace epdiy_epaper {

static const char *TAG = "epdiy_epaper";

void EpdiyEpaper::setup() {
  ESP_LOGCONFIG(TAG, "Setting up epdiy e-paper display...");

  // Initialize epdiy with LilyGo T5 4.7 board, ED047TC1 display
  // Use 64K LUT for full grayscale, 8-line queue for lower memory
  epd_init(&epd_board_lilygo_t5_47, &ED047TC1, (EpdInitOptions)(EPD_LUT_64K | EPD_FEED_QUEUE_8));

  // Get display dimensions
  this->width_ = epd_width();
  this->height_ = epd_height();

  ESP_LOGD(TAG, "Display dimensions: %dx%d", this->width_, this->height_);

  // Initialize high-level state
  this->hl_state_ = epd_hl_init(EPD_BUILTIN_WAVEFORM);

  // Get framebuffer
  this->framebuffer_ = epd_hl_get_framebuffer(&this->hl_state_);

  if (this->framebuffer_ == nullptr) {
    ESP_LOGE(TAG, "Failed to allocate framebuffer!");
    Component::mark_failed();
    return;
  }

  // Clear the display
  epd_poweron();
  epd_fullclear(&this->hl_state_, 25);
  epd_poweroff();

  this->initialized_ = true;
  ESP_LOGCONFIG(TAG, "epdiy display initialized successfully");
}

void EpdiyEpaper::dump_config() {
  LOG_DISPLAY("", "epdiy E-Paper", this);
  ESP_LOGCONFIG(TAG, "  Board: %s", this->board_type_.c_str());
  ESP_LOGCONFIG(TAG, "  Display Type: %s", this->display_type_.c_str());
  ESP_LOGCONFIG(TAG, "  Resolution: %dx%d", this->width_, this->height_);
}

void EpdiyEpaper::update() {
  if (!this->initialized_) {
    return;
  }

  // Clear framebuffer to white
  memset(this->framebuffer_, 0xFF, this->width_ / 2 * this->height_);

  // Call the lambda to draw
  this->do_update_();

  // Update the display
  epd_poweron();

  enum EpdDrawError err = epd_hl_update_screen(&this->hl_state_, MODE_GC16, 25);
  if (err != EPD_DRAW_SUCCESS) {
    ESP_LOGW(TAG, "Display update failed with error: %d", err);
  }

  epd_poweroff();
}

void EpdiyEpaper::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || x >= this->width_ || y < 0 || y >= this->height_) {
    return;
  }

  if (this->framebuffer_ == nullptr) {
    return;
  }

  // Convert color to grayscale (0-255)
  uint8_t gray = color.white;
  if (color.r != 0 || color.g != 0 || color.b != 0) {
    // Use luminance formula if RGB values provided
    gray = (uint8_t)(0.299f * color.r + 0.587f * color.g + 0.114f * color.b);
  }

  // epdiy uses 4-bit grayscale (16 levels), packed 2 pixels per byte
  // Higher value = lighter (white = 0xF, black = 0x0)
  uint8_t gray_4bit = gray >> 4;  // Convert 8-bit to 4-bit

  int idx = y * (this->width_ / 2) + x / 2;
  if (x % 2 == 0) {
    // Even pixel: upper nibble
    this->framebuffer_[idx] = (this->framebuffer_[idx] & 0x0F) | (gray_4bit << 4);
  } else {
    // Odd pixel: lower nibble
    this->framebuffer_[idx] = (this->framebuffer_[idx] & 0xF0) | gray_4bit;
  }
}

int EpdiyEpaper::get_width_internal() { return this->width_; }

int EpdiyEpaper::get_height_internal() { return this->height_; }

}  // namespace epdiy_epaper
}  // namespace esphome
