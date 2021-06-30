#ifndef SSD1306_DISPLAY_H
#define SSD1306_DISPLAY_H

#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

extern Adafruit_SSD1306 display;

bool SSD1306_init(void);  // 液晶ディスプレイ初期化


#endif
