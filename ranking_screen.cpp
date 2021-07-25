#include "ranking_screen.h"

#include "tetris_display.h"
#include "input.h"
#include "SSD1306_display.h"
#include "tetris_bmp.h"

#define BLINK_PERIOD  0.5     // スコア点滅周期
#define SCORE_DIGITS_COUNT  5 // スコア表示桁数


// ランキング画面の1フレーム処理
bool ranking_screen(unsigned long frame_count, unsigned int fps, unsigned int score, unsigned int ranking_scores[], char ranking_count) {
  static char update_rank;

  int x = 70, y = 22;

  if (frame_count == 0) {
    // 最初のフレームではランキング更新を行う
    // 更新する順位を探索
    update_rank = ranking_count;  // 更新する順位（更新しない場合、ranking_countと等しい）
    for (int i = 0; i < ranking_count; i++) {
      if (ranking_scores[i] <= score) {
        update_rank = i;
        break;
      }
    }
    // ランキングを更新
    for (int i = ranking_count - 1; i > update_rank; i--) {
      ranking_scores[i] = ranking_scores[i - 1];  // 更新するスコア以下のものは1つずつ順位をずらしていく
    }
    ranking_scores[update_rank] = score;          // 新スコアで更新

    // ランキング表示
    display.drawBitmap(x + 16, y - 9, RANKING_BMP, RANKING_BMP_WIDTH, RANKING_BMP_HEIGHT, SSD1306_WHITE);
    display.drawLine(x + 14, y - 9, x + 14, y - 9 + 32, SSD1306_WHITE);
    for (int i = 0; i < ranking_count; i++) {
      draw_number(x - i * 14, y - 10, i + 1, 1);
      display.fillRect(x - i * 14, y - 5, 1, 1, SSD1306_WHITE);
      draw_number(x - i * 14, y, ranking_scores[i], SCORE_DIGITS_COUNT);
    }
    display.display();
  } else {
    if (update_rank < ranking_count) {  // ランキングが更新されている場合
      // そのスコアを点滅表示する
      if (frame_count % (int)(fps * BLINK_PERIOD) == 0) {
        draw_number(x - update_rank * 14, y, ranking_scores[update_rank], SCORE_DIGITS_COUNT);
        display.display();
      } else if (frame_count % (int)(fps * BLINK_PERIOD) == (int)(fps * BLINK_PERIOD / 2)){
        display.fillRect(x - update_rank * 14, y, 7, DIGIT_BMP_HEIGHT * SCORE_DIGITS_COUNT, SSD1306_BLACK);
        display.display();
      }
    }
  }

  // 何かボタンが押されたら次の画面に遷移
  if (button_up(BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT | BUTTON_CENTER)) {
    return true;
  }
  return false;
}
