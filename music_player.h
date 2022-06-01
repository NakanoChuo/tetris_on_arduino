#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <Arduino.h>


class MusicPlayer {
private:
  // 1音
  typedef struct {
    unsigned value    : 3;  // 音価　　　　　0:倍音符、1:全音符、2:2分音符、3:4分音符、...、7:64分音符
    unsigned padding1 : 1;
    unsigned dot      : 2;  // 付点　　　　　0:付点なし(1倍)、1:付点(1.5倍)、2:2付点(1.75倍)、3:3付点(1.875倍)
    unsigned slur     : 1;  // スラー　　　　0:なし、1:あり
    unsigned padding2 : 1;
    unsigned pitch    : 4;  // 音階　　　　　0:休符、1:ド、2:ド#、3:レ、...、12:シ、13~:終了コード
    signed octave     : 4;  // オクターブ　　-8~-1:低オクターブ、0:オクターブ変動なし、1~7:高オクターブ
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
