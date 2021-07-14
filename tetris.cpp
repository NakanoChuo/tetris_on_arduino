#include "tetris.h"
#include "tetris_common.h"
#include "mino_shape.h"
#include "tetris_display.h"

#include "input.h"
#include "SSD1306_display.h"


/*
 * 定数
 */
// テトリス動作
#define DROP_PERIOD         0.25  /* 何秒に1回下方向に1ブロック動くか */
#define MOVE_PERIOD         0.05  /* 連続入力したときに何秒に1回横方向に1ブロック動くか */
#define DROP_DISTANCE       1     /* 1フレーム当たりの落下マス数 */
#define FAST_DROP_DISTANCE  5     /* 落下ボタンを押したときの落下マス数 */


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
static block_state get_block(const block_state(*field)[FIELD_WIDTH], int x, int y);             // 指定した座標のブロックの状態を取得
static bool check_collision(                                                                    // ミノの衝突判定
  const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y
);
static int check_move(                                                                          // ミノが横方向あるいは縦方向に動けるかチェック
  const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], 
  int x, int y, int move_distance, int vertical_or_horizontal
);
static bool check_mino_in_field(const byte (*mino)[MINO_SIZE], const mino_info *mino_info);     // ミノがフィールド内か判定
static void fix_mino(                                                                           // ミノをブロックに固定する
  block_state field[FIELD_HEIGHT][FIELD_WIDTH], 
  const byte (*mino)[MINO_SIZE], mino_info *mino_info
);
static int delete_blocks(block_state field[FIELD_HEIGHT][FIELD_WIDTH], int low_y, int high_y);  // ブロックの消去処理



// テトリスゲーム初期化
static void init_tetris(void) {
  is_gameover = false;
  score = 0;
  init_field(field);
  player.state = BLOCK_NONE;
  next.state = BLOCK_NONE;
}


// テトリスの1フレーム処理
bool tetris(unsigned long frame_count, unsigned int fps) {
  bool is_updated = false;      // 画面更新するかどうか

  if (frame_count == 0) {
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
  if (button_continue_press() && (frame_count % (int)(fps * MOVE_PERIOD) == 0)) {
    if (button_press(BUTTON_LEFT)) {
      dx = -1;
    }
    if (button_press(BUTTON_RIGHT)) {
      dx = 1;
    }
  }
  if (button_down(BUTTON_DOWN)) {
    dy = FAST_DROP_DISTANCE;  // 下ボタンが押されたら数マス下に落とす
  } else if (frame_count % (int)(fps * DROP_PERIOD) == 0) {
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
        if (check_mino_in_field(player_mino, &player)) {              // ミノがフィールド内にいるなら
          fix_mino(field, player_mino, &player);                      // ミノをブロックに固定する
          
          int delete_row_count = delete_blocks(field, player.y, player.y + MINO_SIZE);  // 消去した列数
          if (delete_row_count > 0) {
            // スコア計算
            int d_score = 1;  // スコア増分
            for (int i = 0; i < delete_row_count - 1; i++) {
              d_score *= 10;  // 一度に消去した列数に応じて、より多いスコア
            }
            score += d_score;
          }
        } else {                                                      // ミノがフィールド外なら
          is_gameover = true; // ゲームオーバー
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
    draw_field(field);
    draw_mino(&player, player_mino);
    draw_next_mino(&next, next_mino_id);
    draw_score(score);
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


// 指定した座標のブロックの状態を取得
static block_state get_block(const block_state(*field)[FIELD_WIDTH], int x, int y) {
  if (check_field(x, y)) {
    return field[y][x];
  } else if (check_wall(x, y)) {
    return BLOCK_WALL;
  } else {
    return BLOCK_NONE;
  }
}


// ミノがブロックや壁と衝突しているか（重なっているか）チェック
static bool check_collision(const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if (mino[yy][xx] == 1) {
        if (get_block(field, x + xx, y + yy) != BLOCK_NONE) {  // ミノを配置した先に何かあったら
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


// ミノがフィールド内か判定
static bool check_mino_in_field(const byte (*mino)[MINO_SIZE], const mino_info *mino_info) {
  int x = mino_info->x;
  int y = mino_info->y;
  
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if (mino[yy][xx] == 1) {
        if (!check_field(x + xx, y + yy)) {
          return false;
        }
      }
    }
  }

  return true;
}


// ミノをブロックに固定する
static void fix_mino(block_state field[FIELD_HEIGHT][FIELD_WIDTH], const byte (*mino)[MINO_SIZE], mino_info *mino_info) {
  int x = mino_info->x;
  int y = mino_info->y;
  
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if (mino[yy][xx] == 1) {
        field[y + yy][x + xx] = BLOCK_FIXED;  // ミノがあるマスを固定ブロックに変更
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
