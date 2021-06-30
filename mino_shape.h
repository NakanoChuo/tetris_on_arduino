#ifndef MINO_SHAPE_H
#define MINO_SHAPE_H


#include <Arduino.h>

#define MINO_TYPE_COUNT 7 // ミノの種類の数
#define MINO_SIZE 4 // ミノのサイズ（縦横何マス）

// ミノの形状
extern const byte mino_shapes[MINO_TYPE_COUNT][MINO_SIZE][MINO_SIZE] PROGMEM;


#endif
