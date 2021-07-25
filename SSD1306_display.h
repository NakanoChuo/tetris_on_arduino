#ifndef SSD1306_DISPLAY_H
#define SSD1306_DISPLAY_H

#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

extern Adafruit_SSD1306 display;
bool SSD1306_init(void);  // 液晶ディスプレイ初期化

// 数字ビットマップの大きさ
#define DIGIT_BMP_HEIGHT  5
#define DIGIT_BMP_WIDTH   6

void draw_number(int x, int y, unsigned int number, unsigned int digits_count); // 数字を描画
void draw_digit(int x, int y, unsigned int digit);                              // 1桁数字を描画


#endif
