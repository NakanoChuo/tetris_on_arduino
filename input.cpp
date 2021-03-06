#include "input.h"
#include <Arduino.h>


// ボタンのピン番号
#define UP_BUTTON_PIN     4
#define DOWN_BUTTON_PIN   2
#define LEFT_BUTTON_PIN   3
#define RIGHT_BUTTON_PIN  5
#define CENTER_BUTTON_PIN 6


// このフレーム数以上ボタンを押しているなら連続入力しているとする
#define CONTINUE_PRESS_FRAME_COUNT  250


static byte input;               // ボタンの押下状態（ボタンが押されているなら対応するビットが立つ）
static byte prev_input;          // 1つ前の押下状態
static byte press_change_input;  // 押下状態が変化したときの押下状態
static byte press_frame_count;   // ボタン押しているフレーム数
static byte rising_edge_input;   // ボタンが立ち上がりエッジになっているなら対応するビットが立つ
static byte falling_edge_input;  // 立ち下がりエッジ


// 入力ポート初期化
void input_init(void) {
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CENTER_BUTTON_PIN, INPUT_PULLUP);
}


// ボタンの読み取り（定期的に呼び出す）
void update_button_input(void) {
    input = read_button_input();
    rising_edge_input = input & ~prev_input;
    falling_edge_input = ~input & prev_input;
    if (input != prev_input) {
      press_change_input = input;
      press_frame_count = 0;
    }
    prev_input = input;

    if ((input != 0) && (input == prev_input)) {
      press_frame_count++;
    }
}


// ボタンの押下状態を返す
byte read_button_input(void) {
  return 
    ((digitalRead(UP_BUTTON_PIN) == LOW) * BUTTON_UP)
    | ((digitalRead(DOWN_BUTTON_PIN) == LOW) * BUTTON_DOWN)
    | ((digitalRead(LEFT_BUTTON_PIN) == LOW) * BUTTON_LEFT)
    | ((digitalRead(RIGHT_BUTTON_PIN) == LOW) * BUTTON_RIGHT)
    | ((digitalRead(CENTER_BUTTON_PIN) == LOW) * BUTTON_CENTER);
}


// ボタンを押しているかどうか
bool button_press(byte button_code) {
  return input & button_code;
}

// ボタンを押した瞬間かどうか
bool button_down(byte button_code) {
  return rising_edge_input & button_code;
}

// ボタンを離した瞬間かどうか
bool button_up(byte button_code) {
  return falling_edge_input & button_code;
}

// いずれかのボタンが連続入力されているかどうか
bool button_continue_press(void) {
  return press_frame_count >= CONTINUE_PRESS_FRAME_COUNT;
}
