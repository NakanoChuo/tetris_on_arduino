#include "input.h"
#include <Arduino.h>


// ボタンのピン番号
#define UP_BUTTON_PIN     2
#define DOWN_BUTTON_PIN   3
#define LEFT_BUTTON_PIN   4
#define RIGHT_BUTTON_PIN  5


static unsigned int input;              // ボタンの押下状態（ボタンが押されているなら対応するビットが立つ）
static unsigned int prev_input;         // 1つ前の押下状態
static unsigned int rising_edge_input;  // ボタンが立ち上がりエッジになっているなら対応するビットが立つ
static unsigned int falling_edge_input; // 立ち下がりエッジ
static unsigned int hold_input;


// 入力ポート初期化
void input_init(void) {
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
}


// ボタンの読み取り（定期的に呼び出す）
void update_button_input(void) {
    input = read_button_input();
    rising_edge_input = input & ~prev_input;
    falling_edge_input = ~input & prev_input;
    prev_input = input;
}


// ボタンの押下状態を返す
unsigned int read_button_input(void) {
  return 
    ((digitalRead(UP_BUTTON_PIN) == LOW) * BUTTON_UP)
    | ((digitalRead(DOWN_BUTTON_PIN) == LOW) * BUTTON_DOWN)
    | ((digitalRead(LEFT_BUTTON_PIN) == LOW) * BUTTON_LEFT)
    | ((digitalRead(RIGHT_BUTTON_PIN) == LOW) * BUTTON_RIGHT);
}


// ボタンを押しているかどうか
bool button_press(unsigned int button_code) {
  return input & button_code;
}

// ボタンを押した瞬間かどうか
bool button_down(unsigned int button_code) {
  return rising_edge_input & button_code;
}

// ボタンを離した瞬間かどうか
bool button_up(unsigned int button_code) {
  return falling_edge_input & button_code;
}
