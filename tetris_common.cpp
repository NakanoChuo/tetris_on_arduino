#include "tetris_common.h"


// フィールド内か判定
bool check_field(int x, int y) {
  return (0 <= x) && (x < FIELD_WIDTH) && (0 <= y) && (y < FIELD_HEIGHT);
}


// フィールドより外のマスに壁があるか判定
bool check_wall(int x, int y) {
  if (check_field(x, y)) {
    return false;           // フィールド内には壁はない
  }
  if ((y < 0) && (x >= (FIELD_WIDTH - STANDBY_AREA_WIDTH) / 2) && ((FIELD_WIDTH + STANDBY_AREA_WIDTH) / 2 > x)) {
    return false;           // ミノ待機エリアにも壁はない
  }
  return true;              // フィールドとミノ待機エリア以外は全て壁
}
