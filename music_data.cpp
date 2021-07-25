#include "music_data.h"


#include <Arduino.h>

const unsigned int bpm = 92;
const word notes[] PROGMEM = {
  // 高さ（8ビット）、オクターブ（3ビット）、スラー（1ビット）、付点（1ビット）、音価（3ビット）
  0x0004, 0x0204, 0x040B, 0x0204, 0x0003,
  0x0004, 0x0204, 0x0404, 0x0204, 0x0004, 0x0214, 0x0202,

  0xFFFF  // 終了コード
};
