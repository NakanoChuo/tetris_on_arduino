#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <Arduino.h>


class MusicPlayer {
private:
  int bpm = -1;
  const word *notes;        // メロディ
  bool is_playing = false;  // 再生中かどうか
  bool is_sounding = false; // 音符／休符を出力中か
  unsigned long sound_end_time; // 1音符／休符をいつまで鳴らすか[ms]
  bool is_looped = false;   // ループ再生かどうか
  int note_index = 0;       // 現在、メロディ中のどの音を鳴らしているか

public:
  MusicPlayer(void);

public:
  void set_notes(int bpm, const word notes[]);  // BPMとメロディの設定
  void play(bool is_looped);  // 再生
  void restart(void);         // 初めから再生
  void stop(void);            // 停止
  void pause(void);           // 一時停止

  // 音の出力状態を更新する
  // この関数を定期的に呼び出してください
  void update_sound(void);  

private:
  word read_note(void); // 音符データの読み込み
  void sound_note(float note_duration, unsigned int frequency, bool is_slur, bool is_rest); // 音符／休符を出力する
};


#endif
