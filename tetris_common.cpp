#include "tetris_common.h"


// フィールド内か判定
bool check_field(int x, int y) {
  return (0 <= x) && (x < FIELD_WIDTH) && (0 <= y) && (y < FIELD_HEIGHT);
}


// フィールドより外のマスに壁があるか判定
bool check_wall(int x, int y) {
  if (check_field(x, y)) {  // フィールド内には壁はない
    return false;
  }
  if (y < 0) {              // ミノ待機エリアには
    if (y == -1) {          // フィールドとの境界の一部を除き
      if ((x < (FIELD_WIDTH - STANDBY_AREA_WIDTH) / 2) || ((FIELD_WIDTH + STANDBY_AREA_WIDTH) / 2 <= x)) {
        return true;
      }
    }
    return false;           // 壁はない
  }
  return true;              // フィールドとミノ待機エリア以外は全て壁
}
