#include "tetris_display.h"
#include "tetris_common.h"
#include "tetris_bmp.h"

#include "SSD1306_display.h"


// マスのサイズ
#define BLOCK_PX            5

// スコア表示桁数
#define SCORE_DIGITS_COUNT  6


// 壁の描画
void draw_wall() {
  for (int y = -STANDBY_AREA_HEIGHT; y <= FIELD_HEIGHT; y++) {
    for (int x = -1; x <= FIELD_WIDTH; x++) {
      if (check_wall(x, y)) {
        draw_block(x, y, BLOCK_WALL);
      }
    }
  }
}


// スコア欄の描画
void draw_score_bmp() {
  display.drawBitmap(0, 0, score_bmp, SCORE_BMP_WIDTH, SCORE_BMP_HEIGHT, SSD1306_WHITE);
}


// スコアの描画
void draw_score(unsigned int score) {
  display.fillRect(0, SCORE_BMP_HEIGHT, DIGIT_BMP_WIDTH, DIGIT_BMP_HEIGHT * SCORE_DIGITS_COUNT, SSD1306_BLACK);
  draw_number(0, SCORE_BMP_HEIGHT, score, SCORE_DIGITS_COUNT);
}


// ミノ待機エリアとフィールドの描画
void draw_field(const block_state field[FIELD_HEIGHT][FIELD_WIDTH]) {
  // ミノ待機エリアの描画
  for (int y = -STANDBY_AREA_HEIGHT; y < 0; y++) {
    for (int x = 0; x < STANDBY_AREA_WIDTH; x++) {
      draw_block(x + (FIELD_WIDTH - STANDBY_AREA_WIDTH) / 2, y, BLOCK_NONE);
    }
  }
  // フィールドの描画
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      draw_block(x, y, field[y][x]);
    }
  }
}


// プレイヤーのミノの描画
void draw_mino(const mino_info *mino_info, const byte (*mino)[MINO_SIZE]) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if ((mino_info->state != BLOCK_NONE) && (mino[yy][xx] == 1)) {
        draw_block(mino_info->x + xx, mino_info->y + yy, mino_info->state);
      }
    }
  }
}


// 次のミノの描画
void draw_next_mino(const mino_info *mino_info, int mino_id) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if ((mino_info->state != BLOCK_NONE) && (pgm_read_byte(&(mino_shapes[mino_id][yy][xx])) == 1)) {
        draw_block(mino_info->x + xx, mino_info->y + yy, mino_info->state);
      }
    }
  }
}


// マスの描画
void draw_block(int x, int y, block_state state) {
  float display_x, display_y;
  field2display_coord(x, y, &display_x, &display_y);

  int left = display_x - (BLOCK_PX / 2.0);
  int right = display_x + (BLOCK_PX / 2.0) - 1;
  int top = display_y - (BLOCK_PX / 2.0);
  int bottom = display_y + (BLOCK_PX / 2.0) - 1;
  
  switch (state) {
  case BLOCK_WALL:  // マスが壁なら
    display.drawLine(left, top, right, bottom, SSD1306_WHITE);  // 斜線を描画
    break;
  case BLOCK_FIXED: // マスがブロックなら
    display.fillRect(left, top, right - left, bottom - top, SSD1306_WHITE); // 塗りつぶした矩形を描画
    break;
  case BLOCK_MOVABLE: // マスがミノなら
    display.drawRect(left, top, right - left, bottom - top, SSD1306_WHITE); // 白抜きの矩形を描画
    break;
  case BLOCK_NONE:  // マスに何もないなら
  default:
    display.fillRect(left, top, right - left, bottom - top, SSD1306_BLACK); // 黒く塗りつぶす
    break;
  }
}


// マス座標からディスプレイ座標に変換
void field2display_coord(int field_x, int field_y, float *display_x, float *display_y) {
  *display_x = (-field_y + 0.5 + FIELD_HEIGHT + 1) * BLOCK_PX + 2;
  *display_y = (field_x + 0.5 + 1) * BLOCK_PX;
}
