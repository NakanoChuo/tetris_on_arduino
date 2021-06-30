#include "tetris.h"
#include "mino_shape.h"

#include "input.h"
#include "SSD1306_display.h"


// フィールドのサイズ
#define FIELD_WIDTH   12
#define FIELD_HEIGHT  21

static const int BLOCK_PX = 5;  // マスのサイズ

// マスの状態
typedef byte block_state;
static const block_state BLOCK_NONE = 0;    // 何もないマス
static const block_state BLOCK_WALL = 1;    // 壁
static const block_state BLOCK_FIXED = 2;   // ブロック
static const block_state BLOCK_MOVABLE = 3; // 動かせるミノ

// ミノの情報
typedef struct {
  int x, y; // 位置
  block_state state;  // 状態
} mino_info;


static void init_field(block_state field[FIELD_HEIGHT][FIELD_WIDTH]);                           // フィールドの初期化
static void copy_next_mino(byte dst[MINO_SIZE][MINO_SIZE], int mino_id);                        // 番号mino_idで指定したミノの形状を配列にコピー
static void copy_mino(byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE]);            // ミノの配列から配列にコピー
static void rotate_mino(                                                                        // ミノを回転
  byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE], int clockwise
);
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
static void draw_field(const block_state field[FIELD_HEIGHT][FIELD_WIDTH]);                     // フィールドの描画
static void draw_mino(const mino_info *mino_info, const byte (*mino)[MINO_SIZE]);               // プレイヤーのミノの描画
static void draw_next_mino(const mino_info *mino_info, int mino_id);                            // 次のミノの描画
static void draw_block(int x, int y, block_state state);                                        // マスの描画
static void field2display_coord(int field_x, int field_y, float *display_x, float *display_y);  // マス座標からディスプレイ座標に変換


static const float MOVE_SPEED = 0.1;  // 何秒に1回横方向に1ブロック動くか
static const float DROP_SPEED = 0.25; // 何秒に1回下方向に1ブロック動くか

static const int DROP_DISTANCE = 1;       // 1フレーム当たりの落下マス数
static const int FAST_DROP_DISTANCE = 5;  // 下ボタンを押したときの落下マス数


// テトリスの1フレーム処理
bool tetris(unsigned long frame_count, unsigned int fps) {
  bool is_updated = false;  // 画面更新するかどうか
  
  static block_state field[FIELD_HEIGHT][FIELD_WIDTH];  // テトリスのフィールド

  static mino_info player = {0, 0, BLOCK_NONE};   // プレイヤーが動かすミノの状態
  static byte player_mino[MINO_SIZE][MINO_SIZE];  // ミノの形状
  
  static mino_info next = {0, 0, BLOCK_NONE};     // 次のミノの状態
  static int next_mino_id;                        // 次のミノの種類

  static int is_gameover; // ゲームオーバーかどうか

  if (frame_count == 0) { // ゲーム開始直後のリセット処理
    is_gameover = false;
    randomSeed(digitalRead(13));
    init_field(field);
    is_updated = true;
  }

  if (is_gameover) {
    return is_gameover;
  }

  if (next.state != BLOCK_FIXED) {  // 
    // 次のミノの生成
    next.x = (FIELD_WIDTH - MINO_SIZE) / 2;
    next.y = (frame_count == 0) ? 0 : -2;
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
    next.state = BLOCK_NONE;  // 次のミノの状態を初期状態に
    is_updated = true;
  }

  int dx = 0, dy = 0;

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
    next.y += (next.y < 1); // 次のミノも落とす
  }

  if (dx != 0) {
    dx = check_move(field, player_mino, player.x, player.y, dx, 1); // 横方向に動けるかチェック、動ける分だけ動く
    player.x += dx;
    is_updated = true;
  }

  if (dy != 0) {
    dy = check_move(field, player_mino, player.x, player.y, dy, 0); // 下方向に動けるかチェック、動ける分だけ動く
    player.y += dy;
    if (dy == 0) {  // チェックした結果、動けなかったら
      if (player.y <= 2) {  // さらに、上までブロックが積まれていたら
        is_gameover = true; // ゲームオーバー
      }
      fix_mino(field, player_mino, &player);  // ミノをブロックに固定する
      delete_blocks(field, player.y, player.y + MINO_SIZE); // ブロックが消せるかどうかチェック
    }
    is_updated = true;
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
    draw_field(field);  // 積まれているブロックと壁を描画
    draw_mino(&player, player_mino);  // プレイヤーのミノを描画
    draw_next_mino(&next, next_mino_id);  // 次のミノを描画
    display.display();
  }

  return is_gameover;
}


// フィールドの初期化
static void init_field(block_state field[FIELD_HEIGHT][FIELD_WIDTH]) {
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (y == FIELD_HEIGHT - 1) {
        field[y][x] = BLOCK_WALL;
      } else if (y == 3) {
        if (x < 3 || FIELD_WIDTH - 3 <= x) {
          field[y][x] = BLOCK_WALL;
        } else {
          field[y][x] = BLOCK_NONE;
        }
      } else {
        if (x < 1 || FIELD_WIDTH - 1 <= x) {
          field[y][x] = BLOCK_WALL;
        }
      }
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


// ミノがブロックや壁と衝突しているか（重なっているか）チェック
static bool check_collision(const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if (mino[yy][xx] == 1) {
        block_state block = field[y + yy][x + xx];
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
        if ((0 <= y + yy) && (y + yy < FIELD_HEIGHT) && (0 <= x + xx) && (x + xx < FIELD_WIDTH)) {
          field[y + yy][x + xx] = BLOCK_FIXED;  // ミノがあるマスを固定ブロックに変更
        }
      }
    }
  }
  mino_info->state = BLOCK_FIXED; // ミノの状態を固定状態に変更
}


// ブロックの消去処理
static int delete_blocks(block_state field[FIELD_HEIGHT][FIELD_WIDTH], int low_y, int high_y) {
  bool delete_row[FIELD_HEIGHT];  // delete_row[i]はi列が消せるかどうか

  // 初期化
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    delete_row[y] = false;
  }

  // 消去できる列を探していく
  for (int y = max(4, low_y); y < min(FIELD_HEIGHT - 1, high_y); y++) {
    delete_row[y] = true; // 
    for (int x = 1; x < FIELD_WIDTH - 1; x++) { // 1列中のマスについて
      if (field[y][x] == BLOCK_NONE) {  // 1つでも何もないマスがあったら
        delete_row[y] = false;  // その列は削除できない
      }
    }
  }

  // 消去しない列を下に詰めていき、消去する列を上書きする
  int dst_y = FIELD_HEIGHT - 1; // 消去する列番号（上書きされる列）
  int src_y = FIELD_HEIGHT - 1; // 消去しない列番号（上書きする列）
  for (; dst_y >= 4; dst_y--, src_y--) {
    while ((delete_row[src_y]) && (src_y >= 4)) {
      src_y--;
    }
    if (dst_y != src_y) { // 上書きする列と上書きされる列が違うなら上書き処理を行う（同じなら上書き処理をする必要はない）
      // 上書き処理
      for (int x = 1; x < FIELD_WIDTH - 1; x++) {
        if (src_y >= 4) {
          field[dst_y][x] = field[src_y][x];
        } else {
          field[dst_y][x] = BLOCK_NONE;
        }
      }
    }
  }

  return 0;
}


// フィールドの描画
static void draw_field(const block_state field[FIELD_HEIGHT][FIELD_WIDTH]) {
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
  *display_x = (-field_y + 0.5) * BLOCK_PX + BLOCK_PX * (FIELD_HEIGHT + 1);
  *display_y = (field_x + 0.5) * BLOCK_PX;
}
