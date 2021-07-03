#include "tetris.h"
#include "mino_shape.h"

#include "input.h"
#include "SSD1306_display.h"


/*
 * 定数
 */
// フィールドのサイズ
#define FIELD_WIDTH         10
#define FIELD_HEIGHT        20
// 次のミノが待機しているエリア
#define STANDBY_AREA_WIDTH  ((MINO_SIZE) + 2)
#define STANDBY_AREA_HEIGHT 4
// 次のミノの待機位置
#define STANDBY_X           (((FIELD_WIDTH) - (MINO_SIZE)) / 2)
#define STANDBY_Y           (-(MINO_SIZE) + 1)

// マスのサイズ
#define BLOCK_PX            5

// "SCORE:"文字列のビットマップのサイズ
#define SCORE_BMP_HEIGHT    30
#define SCORE_BMP_WIDTH     6

// スコア表示桁数
#define SCORE_DIGITS_COUNT  6

// テトリス動作
#define MOVE_SPEED          0.1   /* 何秒に1回横方向に1ブロック動くか */
#define DROP_SPEED          0.25  /* 何秒に1回下方向に1ブロック動くか */
#define DROP_DISTANCE       1     /* 1フレーム当たりの落下マス数 */
#define FAST_DROP_DISTANCE  5     /* 落下ボタンを押したときの落下マス数 */


/*
 * 型定義
 */
// マスの状態
typedef byte block_state;
static const block_state BLOCK_NONE = 0;    // 何もないマス
static const block_state BLOCK_WALL = 1;    // 壁
static const block_state BLOCK_FIXED = 2;   // 動かせないミノ or ブロック
static const block_state BLOCK_MOVABLE = 3; // 動かせるミノ

// ミノの情報
typedef struct {
  int x, y; // 位置
  block_state state;  // 状態
} mino_info;


/*
 * テトリス情報用変数
 */ 
static bool is_gameover = true;                       // ゲームオーバーかどうか
static unsigned int score;                            // スコア
static block_state field[FIELD_HEIGHT][FIELD_WIDTH];  // テトリスのフィールド
static mino_info player;                              // プレイヤーが動かすミノの状態
static byte player_mino[MINO_SIZE][MINO_SIZE];        // ミノの形状
static mino_info next;                                // 次のミノの状態
static int next_mino_id;                              // 次のミノの種類


/*
 * 関数
 */
static void init_tetris(void);                                                                  // テトリスゲーム初期化
static void init_field(block_state field[FIELD_HEIGHT][FIELD_WIDTH]);                           // フィールドの初期化
static void copy_next_mino(byte dst[MINO_SIZE][MINO_SIZE], int mino_id);                        // 番号mino_idで指定したミノの形状を配列にコピー
static void copy_mino(byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE]);            // ミノの配列から配列にコピー
static void rotate_mino(                                                                        // ミノを回転
  byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE], int clockwise
);
static bool check_field(int x, int y);                                                          // フィールド内か判定
static bool check_wall(int x, int y);                                                           // フィールドより外のマスに壁があるか判定
static bool check_collision(                                                                    // ミノの衝突判定
  const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y
);
static int check_move(                                                                          // ミノが横方向あるいは縦方向に動けるかチェック
  const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], 
  int x, int y, int move_distance, int vertical_or_horizontal
);
static void fix_mino(                                                                           // ミノをブロックに固定する
  block_state field[FIELD_HEIGHT][FIELD_WIDTH], 
  const byte (*mino)[MINO_SIZE], mino_info *mino_info
);
static int delete_blocks(block_state field[FIELD_HEIGHT][FIELD_WIDTH], int low_y, int high_y);  // ブロックの消去処理
static void draw_wall();                                                                        // 壁の描画
static void draw_dynamic_display(void);                                                         // 動的に変化する要素の描画
static void draw_field(const block_state field[FIELD_HEIGHT][FIELD_WIDTH]);                     // ミノ待機エリアとフィールドの描画
static void draw_mino(const mino_info *mino_info, const byte (*mino)[MINO_SIZE]);               // プレイヤーのミノの描画
static void draw_next_mino(const mino_info *mino_info, int mino_id);                            // 次のミノの描画
static void draw_block(int x, int y, block_state state);                                        // マスの描画
static void field2display_coord(int field_x, int field_y, float *display_x, float *display_y);  // マス座標からディスプレイ座標に変換
static void draw_score();                                                                       // スコア欄の描画


// テトリスゲーム初期化
static void init_tetris(void) {
  is_gameover = false;
  score = 0;
  init_field(field);
  player.state = BLOCK_NONE;
  next.state = BLOCK_NONE;
  
  draw_wall();
  draw_score();
}


// テトリスの1フレーム処理
bool tetris(unsigned long frame_count, unsigned int fps) {
  static bool is_first = true;  // 初回フレーム
  bool is_updated = false;      // 画面更新するかどうか

  if (is_first) {
    is_first = false;
    init_tetris();
    is_updated = true;
  }
  
  if (is_gameover) {
    return is_gameover;
  }

  if (next.state == BLOCK_NONE) { // 次のミノがなかったら 
    // 次のミノの生成
    next.x = STANDBY_X;
    next.y = (frame_count == 0) ? STANDBY_Y : STANDBY_Y - MINO_SIZE;
    next.state = BLOCK_FIXED;
    next_mino_id = random(0, MINO_TYPE_COUNT);
    is_updated = true;
  }

  if (player.state != BLOCK_MOVABLE) {  // プレイヤーのミノが動かせない場合
    // 次のミノをプレイヤーのミノに
    player.x = next.x;
    player.y = next.y;
    player.state = BLOCK_MOVABLE;
    copy_next_mino(player_mino, next_mino_id);
    next.state = BLOCK_NONE;  // 次のミノを消す
    is_updated = true;
  }

  int dx = 0, dy = 0;   // プレイヤーのミノの移動量
  int next_mino_dy = 0; // 次のミノの移動量

  // ミノの移動
  if (button_down(BUTTON_LEFT)) {
    dx = -1;
  }
  if (button_down(BUTTON_RIGHT)) {
    dx = 1;
  }
  if (button_down(BUTTON_DOWN)) {
    dy = FAST_DROP_DISTANCE;  // 下ボタンが押されたら数マス下に落とす
  } else if (frame_count % (int)(fps * DROP_SPEED) == 0) {
    dy = DROP_DISTANCE; // 数フレームに1回、少し下に落とす
    next_mino_dy = DROP_DISTANCE; // 次のミノも落とす
  }

  if (player.state == BLOCK_MOVABLE) {
    if (dx != 0) {
      dx = check_move(field, player_mino, player.x, player.y, dx, 1); // 横方向に動けるかチェック、動ける分だけ動く
      player.x += dx;
      is_updated = true;
    }
  
    if (dy != 0) {
      dy = check_move(field, player_mino, player.x, player.y, dy, 0); // 下方向に動けるかチェック、動ける分だけ動く
      player.y += dy;
      if (dy == 0) {  // チェックした結果、動けなかったら
        if (player.y <= STANDBY_Y) {  // さらに、上までブロックが積まれていたら
          is_gameover = true; // ゲームオーバー
        } else {
          fix_mino(field, player_mino, &player);  // ミノをブロックに固定する
          
          int delete_row_count = delete_blocks(field, player.y, player.y + MINO_SIZE);  // 消去した列数
          if (delete_row_count > 0) {
            // スコア計算
            int d_score = 1;  // スコア増分
            for (int i = 0; i < delete_row_count - 1; i++) {
              d_score *= 10;  // 一度に消去した列数に応じて、より多いスコア
            }
            score += d_score;
          }
        }
      }
      is_updated = true;
    }
  }

  if ((next.state != BLOCK_NONE) && !is_gameover) {
    if (next_mino_dy != 0) {
      if (next.y < STANDBY_Y) {
        next.y += next_mino_dy;
        is_updated = true;
      }
    }
  }
  
  // ミノの回転
  if (button_down(BUTTON_UP)) {
    byte temp_mino[MINO_SIZE][MINO_SIZE];
    rotate_mino(temp_mino, player_mino, 0); // 試しに回転
    if (check_collision(field, temp_mino, player.x, player.y)) {  // 回転後、衝突判定を行い衝突しなかったら
      copy_mino(player_mino, temp_mino);  // 回転を適用
    }
    is_updated = true;
  }

  if (is_updated) {
    draw_dynamic_display();
    display.display();
  }

  return is_gameover;
}


// フィールドの初期化
static void init_field(block_state field[FIELD_HEIGHT][FIELD_WIDTH]) {
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      field[y][x] = BLOCK_NONE;
    }
  }
}


// 番号mino_idで指定したミノの形状を配列にコピー
static void copy_next_mino(byte dst[MINO_SIZE][MINO_SIZE], int mino_id) {
  for (int i = 0; i < MINO_SIZE; i++) {
    for (int j = 0; j < MINO_SIZE; j++) {
      dst[i][j] = pgm_read_byte(&(mino_shapes[mino_id][i][j]));
    }
  }
}


// ミノの配列から配列にコピー
static void copy_mino(byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE]) {
  for (int i = 0; i < MINO_SIZE; i++) {
    for (int j = 0; j < MINO_SIZE; j++) {
      dst[i][j] = src[i][j];
    }
  }
}


// ミノを回転
static void rotate_mino(byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE], int clockwise) {
  for (int i = 0; i < MINO_SIZE; i++) {
    for (int j = 0; j < MINO_SIZE; j++) {
      if (clockwise) {
        dst[i][j] = src[j][MINO_SIZE - 1 - i];  // 時計回りに回転
      } else {
        dst[i][j] = src[MINO_SIZE - 1 - j][i];  // 反時計回りに回転
      }
    }
  }
}


// フィールド内か判定
static bool check_field(int x, int y) {
  return (0 <= x) && (x < FIELD_WIDTH) && (0 <= y) && (y < FIELD_HEIGHT);
}


// フィールドより外のマスに壁があるか判定
static bool check_wall(int x, int y) {
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


// ミノがブロックや壁と衝突しているか（重なっているか）チェック
static bool check_collision(const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if (mino[yy][xx] == 1) {
        block_state block;
        if (check_field(x + xx, y + yy)) {
          block = field[y + yy][x + xx];
        } else if (check_wall(x + xx, y + yy)) {
          block = BLOCK_WALL;
        } else {
          block = BLOCK_NONE;
        }
        
        if (block != BLOCK_NONE) {  // ミノを配置した先に何かあったら
          return false;             // 衝突している
        }
      }
    }
  }
  return true;
}


// ミノが横方向あるいは縦方向に動けるかチェック、動ける距離（最大move_distance）を返す
static int check_move(const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y, int move_distance, int vertical_or_horizontal) {
  if (move_distance == 0) {
    return 0;
  }
  
  int dx, dy; // 動かす方向
  int sign = (move_distance > 0) ? 1 : -1;
  if (vertical_or_horizontal == 0) {  // 垂直方向に動かす
    dx = 0;
    dy = sign;
  } else {                            // 水平方向に動かす
    dx = sign;
    dy = 0;
  }

  for (int d = 1; d <= abs(move_distance); d++) { // 1マスずつ動かして衝突判定していく
    if (!check_collision(field, mino, x + dx * d, y + dy * d)) {
      return sign * (d - 1);  // 衝突したのならその1マス前までは動かせる
    }
  }
  return move_distance; // 一回も衝突していないのなら最大限動かせる
}


// ミノをブロックに固定する
static void fix_mino(block_state field[FIELD_HEIGHT][FIELD_WIDTH], const byte (*mino)[MINO_SIZE], mino_info *mino_info) {
  int x = mino_info->x;
  int y = mino_info->y;
  
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if (mino[yy][xx] == 1) {
        if (check_field(x + xx, y + yy)) {
          field[y + yy][x + xx] = BLOCK_FIXED;  // ミノがあるマスを固定ブロックに変更
        }
      }
    }
  }
  mino_info->state = BLOCK_FIXED; // ミノの状態を固定状態に変更
}


// ブロックの消去処理
static int delete_blocks(block_state field[FIELD_HEIGHT][FIELD_WIDTH], int low_y, int high_y) {
  bool delete_row[FIELD_HEIGHT];        // delete_row[i]はi列が消せるかどうか
  int delete_row_count = FIELD_HEIGHT;  // 消す列数

  // 消去できる列を探していく
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    delete_row[y] = true;
    for (int x = 0; x < FIELD_WIDTH; x++) { // 1列中のマスについて
      if (field[y][x] == BLOCK_NONE) {      // 1つでも何もないマスがあったら
        delete_row[y] = false;              // その列は削除できない
        delete_row_count--;
        break;
      }
    }
  }

  // 消去しない列を下に詰めていき、消去する列を上書きする
  int dst_y = FIELD_HEIGHT - 1; // 消去する列番号（上書きされる列）
  int src_y = FIELD_HEIGHT - 1; // 消去しない列番号（上書きする列）
  for (; dst_y >= 0; dst_y--, src_y--) {
    while ((delete_row[src_y]) && (src_y >= 0)) {
      src_y--;
    }
    if (dst_y != src_y) { // 上書きする列と上書きされる列が違うなら上書き処理を行う（同じなら上書き処理をする必要はない）
      // 上書き処理
      for (int x = 0; x < FIELD_WIDTH; x++) {
        if (src_y >= 0) {
          field[dst_y][x] = field[src_y][x];
        } else {
          field[dst_y][x] = BLOCK_NONE;
        }
      }
    }
  }

  return delete_row_count;
}


// 壁の描画
static void draw_wall() {
  for (int y = -STANDBY_AREA_HEIGHT; y <= FIELD_HEIGHT; y++) {
    for (int x = -1; x <= FIELD_WIDTH; x++) {
      if (check_wall(x, y)) {
        draw_block(x, y, BLOCK_WALL);
      }
    }
  }
}


// 動的に変化する要素の描画
static void draw_dynamic_display(void) {
  draw_field(field);  // 積まれているブロックと壁を描画
  draw_mino(&player, player_mino);  // プレイヤーのミノを描画
  draw_next_mino(&next, next_mino_id);  // 次のミノを描画
  display.fillRect(0, SCORE_BMP_HEIGHT, DIGIT_BMP_WIDTH, DIGIT_BMP_HEIGHT * SCORE_DIGITS_COUNT, SSD1306_BLACK);
  draw_number(0, SCORE_BMP_HEIGHT, score, SCORE_DIGITS_COUNT);  // スコアを描画
}


// ミノ待機エリアとフィールドの描画
static void draw_field(const block_state field[FIELD_HEIGHT][FIELD_WIDTH]) {
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
static void draw_mino(const mino_info *mino_info, const byte (*mino)[MINO_SIZE]) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if ((mino_info->state != BLOCK_NONE) && (mino[yy][xx] == 1)) {
        draw_block(mino_info->x + xx, mino_info->y + yy, mino_info->state);
      }
    }
  }
}


// 次のミノの描画
static void draw_next_mino(const mino_info *mino_info, int mino_id) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if ((mino_info->state != BLOCK_NONE) && (pgm_read_byte(&(mino_shapes[mino_id][yy][xx])) == 1)) {
        draw_block(mino_info->x + xx, mino_info->y + yy, mino_info->state);
      }
    }
  }
}


// マスの描画
static void draw_block(int x, int y, block_state state) {
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
static void field2display_coord(int field_x, int field_y, float *display_x, float *display_y) {
  *display_x = (-field_y + 0.5 + FIELD_HEIGHT + 1) * BLOCK_PX + 2;
  *display_y = (field_x + 0.5 + 1) * BLOCK_PX;
}


// "SCORE:"文字列のビットマップ
static const byte score_bmp[SCORE_BMP_HEIGHT] PROGMEM = {
  B01001000,
  B10010100,
  B10010100,
  B01100000,
  B00000000,
  B01111000,
  B10000100,
  B10000100,
  B01001000,
  B00000000,
  B01111000,
  B10000100,
  B10000100,
  B01111000,
  B00000000,
  B11111100,
  B00010100,
  B00010100,
  B11101100,
  B00000000,
  B11111100,
  B10010100,
  B10010100,
  B10000100,
  B00000000,
  B00000000,
  B01001000,
  B00000000,
  B00000000,
  B00000000,
};

// スコア欄の描画
static void draw_score() {
  display.drawBitmap(0, 0, score_bmp, SCORE_BMP_WIDTH, SCORE_BMP_HEIGHT, SSD1306_WHITE);
}
