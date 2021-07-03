#include "title_screen.h"
#include "tetris_display.h"

#include "input.h"
#include "SSD1306_display.h"


static bool is_first = true;

// タイトル画面の1フレーム処理
bool title_screen(unsigned long frame_count, unsigned int fps) {
  if (is_first) {
    is_first = false;
    draw_title();
    display.display();
    Serial.println("title1");
  }
  Serial.println(button_press(BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT));
  if (button_press(BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT)) {
    Serial.println("title1.5");
    return true;
  }
  return false;
}
