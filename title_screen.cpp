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

  // 何かボタンが押されたら次の画面に遷移
  if (button_up(BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT | BUTTON_CENTER)) {
    return true;
  }
  return false;
}
