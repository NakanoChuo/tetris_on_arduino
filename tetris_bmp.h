#ifndef TETRIS_BMP_H
#define TETRIS_BMP_H


#include <Arduino.h>

// タイトルロゴのビットマップ
#define TETRIS_LOGO_BMP_HEIGHT  47
#define TETRIS_LOGO_BMP_WIDTH   64
extern const byte TETRIS_LOGO_BMP[] PROGMEM;

// "SCORE:"文字列のビットマップ
#define SCORE_BMP_HEIGHT    30
#define SCORE_BMP_WIDTH     6
extern const byte SCORE_BMP[] PROGMEM;


#endif
