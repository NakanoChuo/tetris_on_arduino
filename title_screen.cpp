#include "title_screen.h"
#include "tetris_display.h"

#include "input.h"
#include "SSD1306_display.h"


static bool is_first = true;

// タイトル画面の1フレーム処理
bool title_screen(unsigned long frame_count, unsigned int fps) {
  if (frame_count == 0) {
    draw_title();
    display.display();
  }
  if (button_press(BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT)) {
    return true;
  }
  return false;
}
