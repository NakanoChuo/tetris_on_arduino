#ifndef TETRIS_COMMON_H
#define TETRIS_COMMON_H


#include "mino_shape.h"

// フィールドのサイズ
#define FIELD_WIDTH         10
#define FIELD_HEIGHT        20
// 次のミノが待機しているエリア
#define STANDBY_AREA_WIDTH  ((MINO_SIZE) + 2)
#define STANDBY_AREA_HEIGHT 4
// 次のミノの待機位置
#define STANDBY_X           (((FIELD_WIDTH) - (MINO_SIZE)) / 2)
#define STANDBY_Y           (-(MINO_SIZE) + 1)

// マスの状態
typedef byte block_state;
#define BLOCK_NONE          0 /* 何もないマス */
#define BLOCK_WALL          1 /* 壁 */
#define BLOCK_FIXED         2 /* 動かせないミノ or ブロック */
#define BLOCK_MOVABLE       3 /* 動かせるミノ */

// ミノの情報
typedef struct {
  int x, y;           // 位置
  block_state state;  // 状態
} mino_info;


bool check_field(int x, int y); // フィールド内か判定
bool check_wall(int x, int y);  // フィールドより外のマスに壁があるか判定


#endif
