#include "finish_screen.h"
#include "tetris_common.h"
#include "tetris_display.h"

#include "SSD1306_display.h"


#define ANIMATION_PERIOD    0.001 /* 何秒に1回アニメーションさせるか */
#define DELETE_BLOCK_SPEED  3     /* 1フレームに消すブロック数 */

// 終了画面の1フレーム処理
bool finish_screen(unsigned long frame_count, unsigned int fps) {
  static int i;

  if (frame_count == 0) {
    i = 0;
  }
  
  if (frame_count % (int)(fps * ANIMATION_PERIOD) == 0) {
    int x, y;
    for (int j = i - DELETE_BLOCK_SPEED; j <= i; j++) {
      if (j >= 0) {
        if (j < FIELD_WIDTH * FIELD_HEIGHT) {
          x = j % FIELD_WIDTH;
          y = FIELD_HEIGHT - 1 - j / FIELD_WIDTH;
        } else {
          int k = j - FIELD_WIDTH * FIELD_HEIGHT;
          if (k < STANDBY_AREA_WIDTH * STANDBY_AREA_HEIGHT) {
            x = k % STANDBY_AREA_WIDTH + (FIELD_WIDTH - STANDBY_AREA_WIDTH) / 2;
            y = -1 - (k / STANDBY_AREA_WIDTH);
          } else {
            return true;
          }
        }
        if (j < i) {
          draw_block(x, y, BLOCK_NONE);
        } else {
          draw_block(x, y, BLOCK_FIXED);
        }
      }
    }
    display.display();
    i += DELETE_BLOCK_SPEED;
  }
  
  return false;
}
