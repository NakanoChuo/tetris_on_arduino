#include <EEPROM.h>

#include "tetris_display.h"
#include "title_screen.h"
#include "tetris.h"
#include "ranking_screen.h"
#include "finish_screen.h"

#include "input.h"
#include "SSD1306_display.h"
#include "music_player.h"
#include "music_data.h"


static unsigned long frame_count = 0; // 累積フレーム数
static const unsigned int fps = 1000; // 1秒間のフレーム数

#define RANKING_COUNT 3   /* 保存するランキングの順位数 */
static unsigned int ranking_scores[RANKING_COUNT];  // ランキング

#define RANKING_ADDRESS(rank) ((rank) * sizeof(*ranking_scores))              /* ランキングを保存するアドレス */
#define RANDOM_SEED_ADDRESS   ((RANKING_ADDRESS(0)) + sizeof(ranking_scores)) /* 乱数シードを保存するアドレス */

// ゲーム状態
#define GAME_STATE_TITLE    0
#define GAME_STATE_PLAY     1
#define GAME_STATE_RANKING  2
#define GAME_STATE_END      3
static byte state = GAME_STATE_TITLE;

// 音楽再生用
MusicPlayer music;


void setup() {
  // ハードウェア初期化
  Serial.begin(9600);
  input_init();
  if (!SSD1306_init()) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();

  // ディスプレイ初期化
  draw_wall();
  draw_score_bmp();
  draw_score(0);

  // ランキングリセット
  update_button_input();
  if (button_down(BUTTON_CENTER)) { // 起動時に中央ボタンが押されていたらランキングリセット
    for (int i = 0; i < RANKING_COUNT; i++) {
      EEPROM.put(RANKING_ADDRESS(i), (unsigned int)0);
    }
  }
  
  // ランキング読み込み
  for (int i = 0; i < RANKING_COUNT; i++) {
    EEPROM.get(RANKING_ADDRESS(i), ranking_scores[i]);
  }

  // 音楽設定
  music = MusicPlayer();
  music.set_notes(bpm, notes);
  music.play(true);

  // レジスタ設定
  set_register();
}

// タイマー割り込み用レジスタ設定
void set_register() {
  // CTCモード
  TCCR1B |= (1 << WGM12);
  TCCR1A &= ~(1 << WGM11);
  TCCR1A &= ~(1 << WGM10);
  
  // 64分周（16MHz/64）＆カウンタ値=24に設定
  // 1/(16MHz/64)×25=100μsに1回割り込み発生
  TCCR1B &= ~(1 << CS12);
  TCCR1B |= (1 << CS11);
  TCCR1B |= (1 << CS10);
  OCR1A = 24;
  
  // タイマー割り込みを許可
  TIMSK1 |= (1 << OCIE1A);
}

// タイマー割り込み時の処理
ISR(TIMER1_COMPA_vect) {
  music.update_sound(); // 音を鳴らす
}


void loop() {
  update_button_input();  // ボタンの入力状態取得  

  static unsigned int score;
  byte next_state = state;

  // 各ゲーム状態の終了時処理
  switch (state) {
  case GAME_STATE_TITLE:
    if (title_screen(frame_count, fps)) {
      clear_field();
      next_state = GAME_STATE_PLAY;
    }
    break;
  case GAME_STATE_PLAY:
    if (tetris(frame_count, fps, score)) {
      next_state = GAME_STATE_END;
    }
    break;
  case GAME_STATE_END:
    if (finish_screen(frame_count, fps)) {
      clear_field();
      next_state = GAME_STATE_RANKING;
    }
    break;
  case GAME_STATE_RANKING:
    if (ranking_screen(frame_count, fps, score, ranking_scores, RANKING_COUNT)) {
      // ランキング保存
      for (int i = 0; i < RANKING_COUNT; i++) {
        EEPROM.put(RANKING_ADDRESS(i), ranking_scores[i]); 
      }
      clear_field();
      next_state = GAME_STATE_TITLE;
    }
    break;
  default:
    break;
  }

  if (state != next_state) {
    // 各ゲーム状態の開始時処理
    switch (next_state) {
    case GAME_STATE_PLAY:
      // 乱数シード設定
      unsigned long seed;
      EEPROM.get(RANDOM_SEED_ADDRESS, seed);
      seed += millis();
      randomSeed(seed);
      EEPROM.write(RANDOM_SEED_ADDRESS, seed);
    }
    frame_count = 0;
  } else {
    frame_count++;
  }
  
  state = next_state;

  delay((unsigned long)(1000 / fps));
}
