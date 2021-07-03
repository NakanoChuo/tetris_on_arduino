#include "input.h"
#include "SSD1306_display.h"
#include "tetris.h"


static unsigned long frame_count = 0; // 累積フレーム数
static const unsigned int fps = 1000; // 1秒間のフレーム数


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
}


void loop() {
  update_button_input();  // ボタンの入力状態取得
  
  tetris(frame_count, fps); // テトリス1フレーム処理
  frame_count++;

  delay((unsigned long)(1000 / fps));
}
