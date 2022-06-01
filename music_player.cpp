#include "music_player.h"

#include <Arduino.h>

#define BUZZER_PIN  7
#define SOUND_RATE  0.9f  /* 1拍子あたりの音を鳴らす割合（複数の拍子も1音に繋がって聞こえる）*/

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


// 音を出力する
// この関数を定期的に呼び出してください
void MusicPlayer::update_sound(void) {
  static unsigned long end_time;          // 1音符／休符をいつまで鳴らすか保持する
  unsigned long current_time = millis();  // 現在時刻

  static bool prev_is_playing = false;
  if (this->is_playing && !prev_is_playing) { // 今回のフレームからis_playingがtrueになった場合の初期設定
    end_time = current_time;
  }

  if (this->is_playing && this->bpm > 0 && end_time <= current_time) {  // 再生中かつ、メロディが設定済みかつ、1音符／休符が鳴らし終えている場合
    // 新しい音符／休符を読みこみ、音を鳴らす
    unsigned int note_duration; // 音符／休符の期間
    unsigned int frequency;     // 周波数
    bool is_slur, is_rest;

    if (this->read_note(note_duration, frequency, is_slur, is_rest)) {  // 読み込んだ音符／休符が終了コードではないなら、その音を鳴らす
      if (is_rest) {                    // 休符の場合
        noTone(BUZZER_PIN);             // 音を止める
      } else {
        if (is_slur) {                  // スラーの音符の場合
          tone(BUZZER_PIN, frequency);  // 次の音とつなげる
        } else {                        //　通常の音符の場合
          unsigned int sound_duration = note_duration * SOUND_RATE + end_time - current_time; // 音を鳴らす期間
          tone(BUZZER_PIN, frequency, sound_duration);  // 音を鳴らす
        }
      }
      end_time += note_duration;        // 音符／休符を鳴らし終える時刻を設定
      this->note_index++;
    } else {                            // 終了コードが読み込まれた場合
      this->stop();                     // 停止
      if (this->is_looped) {            // ループ再生なら
        this->play(this->is_looped);    // 再び再生
      }
    }
  }

  prev_is_playing = this->is_playing;
}


// 音符／休符の読み込み
bool MusicPlayer::read_note(unsigned int& ret_note_duration, unsigned int& ret_frequency, bool& is_slur, bool& is_rest) {
  word note = pgm_read_word(&(this->notes[this->note_index]));
  MusicPlayer::note *n_ptr = (MusicPlayer::note *)&note;

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
  float note_duration = 1000.0f * 60.0f / this->bpm;
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
    case 0: /* note_duratioin *= 1; */  break;  // ×1
    case 1: note_duration *= 1.5f;      break;  // ×(1 + 1/2)
    case 2: note_duration *= 1.75f;     break;  // ×(1 + 1/2 + 1/4)
    case 3: note_duration *= 1.875f;    break;  // ×(1 + 1/2 + 1/4 + 1/8)
  }
  ret_note_duration = (unsigned int)note_duration;

  return true;
}
