#include "tetris.h"
#include "mino_shape.h"

#include "input.h"
#include "SSD1306_display.h"


#define FIELD_WIDTH   12
#define FIELD_HEIGHT  21

static const int BLOCK_PX = 5;

typedef byte block_state;
static const block_state BLOCK_NONE = 0;
static const block_state BLOCK_WALL = 1;
static const block_state BLOCK_FIXED = 2;
static const block_state BLOCK_MOVABLE = 3;

typedef struct {
  int x, y;
  block_state state;
} mino_info;


static void init_field(block_state field[FIELD_HEIGHT][FIELD_WIDTH]);
static void copy_next_mino(byte dst[MINO_SIZE][MINO_SIZE], int mino_id);
static void copy_mino(byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE]);
static void rotate_mino(byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE], int clockwise);
static bool check_collision(const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y);
static int check_move(const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y, int move_distance, int vertical_or_horizontal);
static void fix_mino(block_state field[FIELD_HEIGHT][FIELD_WIDTH], const byte (*mino)[MINO_SIZE], mino_info *mino_info);
static int delete_blocks(block_state field[FIELD_HEIGHT][FIELD_WIDTH], int low_y, int high_y);
static void draw_field(const block_state field[FIELD_HEIGHT][FIELD_WIDTH]);
static void draw_mino(const mino_info *mino_info, const byte (*mino)[MINO_SIZE]);
static void draw_next_mino(const mino_info *mino_info, int mino_id);
static void draw_block(int x, int y, block_state state);
static void field2display_coord(int field_x, int field_y, float *display_x, float *display_y);


static const float MOVE_SPEED = 0.1;  // 何秒に1回横方向に1ブロック動くか
static const float DROP_SPEED = 0.25;  // 何秒に1回下方向に1ブロック動くか

static const int FAST_DROP_DISTANCE = 5;


bool tetris(unsigned long frame_count, unsigned int fps) {
  bool is_updated = false;
  
  static block_state field[FIELD_HEIGHT][FIELD_WIDTH];

  static mino_info player = {0, 0, BLOCK_NONE};
  static byte player_mino[MINO_SIZE][MINO_SIZE];
  
  static mino_info next = {0, 0, BLOCK_NONE};
  static int next_mino_id;

  static int is_gameover = false;

  if (frame_count == 0) {
    is_gameover = false;
    randomSeed(digitalRead(13));
    init_field(field);
    is_updated = true;
  }

  if (is_gameover) {
    return is_gameover;
  }

  if (next.state != BLOCK_FIXED) {
    // 次のミノの生成
    next.x = (FIELD_WIDTH - MINO_SIZE) / 2;
    next.y = (frame_count == 0) ? 0 : -2;
    next.state = BLOCK_FIXED;
    next_mino_id = random(0, MINO_TYPE_COUNT);
    is_updated = true;
  }

  if (player.state != BLOCK_MOVABLE) {
    player.x = next.x;
    player.y = next.y;
    player.state = BLOCK_MOVABLE;
    copy_next_mino(player_mino, next_mino_id);
    next.state = BLOCK_NONE;
    is_updated = true;
  }

  int dx = 0, dy = 1;

  if (button_down(BUTTON_LEFT)) {
    dx = -1;
  }
  if (button_down(BUTTON_RIGHT)) {
    dx = 1;
  }
  if (button_down(BUTTON_DOWN)) {
    dy = FAST_DROP_DISTANCE;
  }

  if (button_down(BUTTON_UP)) {
    byte temp_mino[MINO_SIZE][MINO_SIZE];
    rotate_mino(temp_mino, player_mino, 0);
    if (check_collision(field, temp_mino, player.x, player.y)) {
      copy_mino(player_mino, temp_mino);
    }
  }

  if (dx != 0) {
    dx = check_move(field, player_mino, player.x, player.y, dx, 1);
    player.x += dx;
  }

  if ((frame_count % (int)(fps * DROP_SPEED) == 0) || (dy > 1)) {
    dy = check_move(field, player_mino, player.x, player.y, dy, 0);
    player.y += dy;
    if (dy == 0) {
      if (player.y <= 2) {
        is_gameover = true;
      }
      fix_mino(field, player_mino, &player);
      delete_blocks(field, player.y, player.y + MINO_SIZE);
    }

    next.y += (next.y < 1);
    
    is_updated = true;
  }

  if (is_updated) {
    draw_field(field);
    draw_mino(&player, player_mino);
    draw_next_mino(&next, next_mino_id);
    display.display();
  }

  return is_gameover;
}


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


static void copy_next_mino(byte dst[MINO_SIZE][MINO_SIZE], int mino_id) {
  for (int i = 0; i < MINO_SIZE; i++) {
    for (int j = 0; j < MINO_SIZE; j++) {
      dst[i][j] = pgm_read_byte(&(mino_shapes[mino_id][i][j]));
    }
  }
}


static void copy_mino(byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE]) {
  for (int i = 0; i < MINO_SIZE; i++) {
    for (int j = 0; j < MINO_SIZE; j++) {
      dst[i][j] = src[i][j];
    }
  }
}


static void rotate_mino(byte dst[MINO_SIZE][MINO_SIZE], const byte (*src)[MINO_SIZE], int clockwise) {
  for (int i = 0; i < MINO_SIZE; i++) {
    for (int j = 0; j < MINO_SIZE; j++) {
      if (clockwise) {
        dst[i][j] = src[j][MINO_SIZE - 1 - i];
      } else {
        dst[i][j] = src[MINO_SIZE - 1 - j][i];
      }
    }
  }
}


static bool check_collision(const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if (mino[yy][xx] == 1) {
        block_state block = field[y + yy][x + xx];
        if (block != BLOCK_NONE) {
          return false;
        }
      }
    }
  }
  return true;
}


static int check_move(const block_state (*field)[FIELD_WIDTH], const byte (*mino)[MINO_SIZE], int x, int y, int move_distance, int vertical_or_horizontal) {
  int dx, dy;
  int sign;
  if (move_distance > 0) {
    sign = 1;
  } else if (move_distance < 0) {
    sign = -1;
  } else {
    sign = 0;
  }
  if (vertical_or_horizontal == 0) {
    dx = 0;
    dy = sign;    
  } else {
    dx = sign;
    dy = 0;
  }

  int d;
  for (d = 1; d <= abs(move_distance); d++) {
    if (!check_collision(field, mino, x + dx * d, y + dy * d)) {
      return sign * (d - 1);
    }
  }

  return move_distance;
}


static void fix_mino(block_state field[FIELD_HEIGHT][FIELD_WIDTH], const byte (*mino)[MINO_SIZE], mino_info *mino_info) {
  int x = mino_info->x;
  int y = mino_info->y;
  
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if (mino[yy][xx] == 1) {
        if ((0 <= y + yy) && (y + yy < FIELD_HEIGHT) && (0 <= x + xx) && (x + xx < FIELD_WIDTH)) {
          field[y + yy][x + xx] = BLOCK_FIXED;
        }
      }
    }
  }

  mino_info->state = BLOCK_FIXED;
}


static int delete_blocks(block_state field[FIELD_HEIGHT][FIELD_WIDTH], int low_y, int high_y) {
  bool delete_row[FIELD_HEIGHT];
  bool aa = false;
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    delete_row[y] = false;
  }
  for (int y = max(4, low_y); y < min(FIELD_HEIGHT - 1, high_y); y++) {
    delete_row[y] = true;
    for (int x = 1; x < FIELD_WIDTH - 1; x++) {
      if (field[y][x] == BLOCK_NONE) {
        delete_row[y] = false;
      }
    }
  }

  int dst_y = FIELD_HEIGHT - 1;
  int src_y = FIELD_HEIGHT - 1;
  for (; dst_y >= 4; dst_y--, src_y--) {
    while ((delete_row[src_y]) && (src_y >= 4)) {
      src_y--;
    }
    if (dst_y != src_y) {
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


static void draw_field(const block_state field[FIELD_HEIGHT][FIELD_WIDTH]) {
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      draw_block(x, y, field[y][x]);
    }
  }
}


static void draw_mino(const mino_info *mino_info, const byte (*mino)[MINO_SIZE]) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if ((mino_info->state != BLOCK_NONE) && (mino[yy][xx] == 1)) {
        draw_block(mino_info->x + xx, mino_info->y + yy, mino_info->state);
      }
    }
  }
}

static void draw_next_mino(const mino_info *mino_info, int mino_id) {
  for (int yy = 0; yy < MINO_SIZE; yy++) {
    for (int xx = 0; xx < MINO_SIZE; xx++) {
      if ((mino_info->state != BLOCK_NONE) && (pgm_read_byte(&(mino_shapes[mino_id][yy][xx])) == 1)) {
        draw_block(mino_info->x + xx, mino_info->y + yy, mino_info->state);
      }
    }
  }
}


static void draw_block(int x, int y, block_state state) {
  float display_x, display_y;
  field2display_coord(x, y, &display_x, &display_y);

  int left = display_x - (BLOCK_PX / 2.0);
  int right = display_x + (BLOCK_PX / 2.0) - 1;
  int top = display_y - (BLOCK_PX / 2.0);
  int bottom = display_y + (BLOCK_PX / 2.0) - 1;
  
  switch (state) {
  case BLOCK_WALL:
    display.drawLine(left, top, right, bottom, SSD1306_WHITE);
    break;
  case BLOCK_FIXED:
    display.fillRect(left, top, right - left, bottom - top, SSD1306_WHITE);
    break;
  case BLOCK_MOVABLE:
    display.drawRect(left, top, right - left, bottom - top, SSD1306_WHITE);
    break;
  case BLOCK_NONE:
  default:
    display.fillRect(left, top, right - left, bottom - top, SSD1306_BLACK);
    break;
  }
}


static void field2display_coord(int field_x, int field_y, float *display_x, float *display_y) {
  *display_x = (-field_y + 0.5) * BLOCK_PX + BLOCK_PX * (FIELD_HEIGHT + 1);
  *display_y = (field_x + 0.5) * BLOCK_PX;
}
