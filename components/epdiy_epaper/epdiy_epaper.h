#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"

// epdiy library headers
extern "C" {
#include "epdiy.h"
}

namespace esphome {
namespace epdiy_epaper {

class EpdiyEpaper : public display::DisplayBuffer {
 public:
  void setup() override;
  void dump_config() override;
  void update() override;

  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  void set_display_type(const std::string &type) { this->display_type_ = type; }
  void set_board_type(const std::string &type) { this->board_type_ = type; }

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_GRAYSCALE; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_width_internal() override;
  int get_height_internal() override;

  std::string display_type_{"ED047TC1"};
  std::string board_type_{"LILYGO_T5_47"};
  EpdiyHighlevelState hl_state_;
  uint8_t *framebuffer_{nullptr};
  int width_{0};
  int height_{0};
  bool initialized_{false};
};

}  // namespace epdiy_epaper
}  // namespace esphome
