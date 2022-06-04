#include "music_player.h"

#include <Arduino.h>

#define BUZZER_PIN  7
#define SOUND_RATE  0.99f  /* 1拍子あたりの音を鳴らす割合（1だと複数の拍子も1音に繋がって聞こえる）*/

// 1音データ
typedef struct {
  unsigned value    : 3;  // 音価　　　　　0:倍音符、1:全音符、2:2分音符、3:4分音符、...、7:64分音符
  unsigned padding1 : 1;
  unsigned dot      : 2;  // 付点　　　　　0:付点なし(1倍)、1:付点(1.5倍)、2:2付点(1.75倍)、3:3付点(1.875倍)
  unsigned slur     : 1;  // スラー　　　　0:なし、1:あり
  unsigned padding2 : 1;
  unsigned pitch    : 4;  // 音階　　　　　0:休符、1:ド、2:ド#、3:レ、...、12:シ、13~:終了コード
  signed octave     : 4;  // オクターブ　　-8~-1:低オクターブ、0:オクターブ変動なし、1~7:高オクターブ
} note_data;

static const unsigned int frequency_list[] PROGMEM = {
  262,  // ド
  277,  // ド♯
  294,  // レ
  311,  // レ♯
  330,  // ミ
  349,  // ファ
  370,  // ファ♯
  392,  // ソ
  415,  // ソ♯
  440,  // ラ
  466,  // ラ♯
  494,  // シ
};
static const char PITCH_COUNT = sizeof(frequency_list) / sizeof(*frequency_list);

static bool parse_note(word note, unsigned int bpm, float& ret_note_duration, unsigned int& ret_frequency, bool& is_slur, bool& is_rest); // 音符／休符の解析


MusicPlayer::MusicPlayer(void) {
}


// BPMとメロディの設定
void MusicPlayer::set_notes(int bpm, const word notes[]) {
  this->bpm = bpm;
  this->notes = notes;
}


// 再生
void MusicPlayer::play(bool is_looped) {
  this->is_looped = is_looped;
  this->is_playing = true;
}


// 初めから再生
void MusicPlayer::restart(void) {
  this->note_index = 0;
  this->is_playing = true;
}


// 停止
void MusicPlayer::stop(void) {
  this->pause();
  this->note_index = 0;
}


// 一時停止
void MusicPlayer::pause(void) {
  this->is_playing = false;
  noTone(BUZZER_PIN);
}


// 音の出力状態を更新する
// この関数を定期的に呼び出してください
void MusicPlayer::update_sound(void) {
  unsigned long current_time = millis();  // 現在時刻
  bool is_sounding = this->sound_end_time >= current_time;  // 音符／休符を出力中かどうか
  
  if (this->is_playing && this->bpm && not is_sounding) {
    while (true) {
      word note = this->read_note();
      
      float note_duration;
      unsigned int frequency;
      bool is_slur, is_rest;

      if (parse_note(note, this->bpm, note_duration, frequency, is_slur, is_rest)) {
        this->sound_note(note_duration, frequency, is_slur, is_rest);
        break;
      } else if (this->is_looped) {
        this->restart();
      } else {
        this->stop();
        break;
      }
    }
  }
}


// 音符データの読み込み
word MusicPlayer::read_note(void) {
  return pgm_read_word(&(this->notes[this->note_index++]));
}


// 音符／休符を出力する
void MusicPlayer::sound_note(float note_duration, unsigned int frequency, bool is_slur, bool is_rest) {
  unsigned long current_time = millis();  // 現在時刻

  static bool prev_frame_is_playing = false;
  if (this->is_playing && not prev_frame_is_playing) { // 今回のフレームからis_playingがtrueになった場合の初期設定
    this->sound_end_time = current_time;
  }
  prev_frame_is_playing = this->is_playing;

  if (is_rest) {                    // 休符の場合
    noTone(BUZZER_PIN);             // 音を止める
  } else {
    if (is_slur) {                  // スラーの音符の場合
      tone(BUZZER_PIN, frequency);  // 次の音とつなげる
    } else {                        //　通常の音符の場合
      unsigned int sound_duration = note_duration * SOUND_RATE + this->sound_end_time - current_time; // 音を鳴らす期間
      tone(BUZZER_PIN, frequency, sound_duration);  // 音を鳴らす
    }
  }
  this->sound_end_time += (unsigned long)note_duration; // 音符／休符を鳴らし終える時刻を設定
}


// 音符／休符の解析
bool parse_note(word note, unsigned int bpm, float& ret_note_duration, unsigned int& ret_frequency, bool& is_slur, bool& is_rest) {
  note_data *n_ptr = (note_data *)&note;

  if (PITCH_COUNT < n_ptr->pitch) { // 異常な音階は終了コードを表す
    return false;
  }
  
  // 休符かどうか
  is_rest = (n_ptr->pitch == 0);
  
  unsigned char pitch = n_ptr->pitch - 1; // 音階
  signed char octave = n_ptr->octave;     // オクターブ
  unsigned char slur = n_ptr->slur;       // スラーかどうか
  unsigned char dot = n_ptr->dot;         // 付点の数
  unsigned char value = n_ptr->value;     // 音価

  // 音階、オクターブから周波数を決定
  if (not is_rest) {
    ret_frequency = pgm_read_word(&frequency_list[pitch]);
    if (octave >= 0) {
      ret_frequency <<= octave;
    } else {
      ret_frequency >>= -octave;
    }
  }

  // スラーかどうか
  is_slur = (slur > 0);

  // 音価と付点から音を鳴らす時間を決定
  float note_duration = 1000.0f * 60.0f / bpm;;
  switch (value) {
  case 0: note_duration *= 8;       break;  // 倍音符
  case 1: note_duration *= 4;       break;  // 全音符
  case 2: note_duration *= 2;       break;  // 2分音符
  case 3: /* note_duration *= 1; */ break;  // 4分音符
  case 4: note_duration /= 2;       break;  // 8分音符
  case 5: note_duration /= 4;       break;  // 16分音符
  case 6: note_duration /= 8;       break;  // 32分音符
  case 7: note_duration /= 16;      break;  // 64分音符
  }
  switch (dot) {
  case 0: /* note_duratioin *= 1; */  break;  // × 1
  case 1: note_duration *= 1.5f;      break;  // ×(1 + 1/2)
  case 2: note_duration *= 1.75f;     break;  // ×(1 + 1/2 + 1/4)
  case 3: note_duration *= 1.875f;    break;  // ×(1 + 1/2 + 1/4 + 1/8)
  }
  ret_note_duration = note_duration;

  return true;
}
