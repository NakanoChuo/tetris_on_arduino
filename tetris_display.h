#ifndef TETRIS_DISPLAY_H
#define TETRIS_DISPLAY_H


#include "tetris_common.h"

void draw_wall(void);                                                                   // 壁の描画
void draw_score_bmp(void);                                                              // スコア欄の描画
void draw_score(unsigned int score);                                                    // スコアの描画
void draw_field(const block_state field[FIELD_HEIGHT][FIELD_WIDTH]);                    // ミノ待機エリアとフィールドの描画
void draw_mino(const mino_info *mino_info, const byte (*mino)[MINO_SIZE]);              // プレイヤーのミノの描画
void draw_next_mino(const mino_info *mino_info, int mino_id);                           // 次のミノの描画
void draw_block(int x, int y, block_state state);                                       // マスの描画
void field2display_coord(int field_x, int field_y, float *display_x, float *display_y); // マス座標からディスプレイ座標に変換


#endif
