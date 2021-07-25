#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <Arduino.h>


class MusicPlayer {
private:
  // 1音
  typedef struct note {
    unsigned value  : 3;  // 音価
    unsigned dot    : 1;  // 付点
    unsigned slur   : 1;  // スラー
    signed octave   : 3;  // オクターブ
    unsigned pitch  : 8;  // 音の高さ
  } note;

private:
  int bpm = -1;
  const word *notes;        // メロディ
  bool is_playing = false;  // 再生中かどうか
  bool is_looped = false;   // ループ再生かどうか
  int note_index = 0;       // 現在、メロディ中のどの音を鳴らしているか

public:
  MusicPlayer(void);

public:
  void set_notes(int bpm, const word notes[]);  // BPMとメロディの設定
  void play(bool is_looped);
  void stop(void);
  void pause(void);

  // 音を出力する
  // この関数を定期的に呼び出してください
  void update_sound(void);  

private:
  bool read_note(unsigned int& ret_note_duration, unsigned int& ret_frequency, bool& is_slur, bool& is_rest); // 音符／休符の読み込み

};


#endif
