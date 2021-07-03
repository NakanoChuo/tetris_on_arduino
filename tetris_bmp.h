#ifndef TETRIS_BMP_H
#define TETRIS_BMP_H


#include <Arduino.h>

// "SCORE:"文字列のビットマップのサイズ
#define SCORE_BMP_HEIGHT    30
#define SCORE_BMP_WIDTH     6
extern const byte score_bmp[SCORE_BMP_HEIGHT] PROGMEM;  // "SCORE:"文字列のビットマップ


#endif
