#include "tetris_display.h"
#include "title_screen.h"
#include "tetris.h"
#include "finish_screen.h"

#include "input.h"
#include "SSD1306_display.h"


static unsigned long frame_count = 0; // 累積フレーム数
static const unsigned int fps = 1000; // 1秒間のフレーム数

#define GAME_STATE_TITLE  0
#define GAME_STATE_PLAY   1
#define GAME_STATE_END    2
static byte state = GAME_STATE_TITLE;


void setup() {
  randomSeed(digitalRead(13));
  
  // ハードウェア初期化
  Serial.begin(9600);
  input_init();
  if (!SSD1306_init()) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();

  // ディスプレイ初期化
  draw_wall();
  draw_score_bmp();
  draw_score(0);
}


void loop() {
  update_button_input();  // ボタンの入力状態取得
  byte next_state = state;

  // 各ゲーム状態の終了時処理
  switch (state) {
  case GAME_STATE_TITLE:
    if (title_screen(frame_count, fps)) {
      clear_field();
      next_state = GAME_STATE_PLAY;
    }
    break;
  case GAME_STATE_PLAY:
    if (tetris(frame_count, fps)) {
      next_state = GAME_STATE_END;
    }
    break;
  case GAME_STATE_END:
    if (finish_screen(frame_count, fps)) {
      clear_field();
      next_state = GAME_STATE_TITLE;
    }
    break;
  default:
    break;
  }

  if (state != next_state) {
    frame_count = 0;
  } else {
    frame_count++;
  }
  state = next_state;

  delay((unsigned long)(1000 / fps));
}
