#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>


// ボタンコード
#define BUTTON_UP      B0001
#define BUTTON_DOWN    B0010
#define BUTTON_LEFT    B0100
#define BUTTON_RIGHT   B1000


void input_init(void);                        // 入力ポート初期化
void update_button_input(void);               // ボタンの読み取り（定期的に呼び出す）
unsigned int read_button_input(void);         // ボタンの押下状態を返す
bool button_press(unsigned int button_code);  // ボタンを押しているかどうか
bool button_down(unsigned int button_code);   // ボタンを押した瞬間かどうか
bool button_up(unsigned int button_code);     // ボタンを離した瞬間かどうか


#endif
