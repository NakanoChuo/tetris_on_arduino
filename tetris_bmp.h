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

// "RANKING"文字列のビットマップ
#define RANKING_BMP_HEIGHT  33
#define RANKING_BMP_WIDTH   6
extern const byte RANKING_BMP[RANKING_BMP_HEIGHT] PROGMEM;


#endif
