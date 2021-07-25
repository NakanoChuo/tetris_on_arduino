#include "SSD1306_display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define OLED_RESET    -1  // sharing Arduino reset pin
#define SCREEN_ADDRESS  0x3C  // I2C address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 数字のビットマップ
static const byte digit_bmp[][DIGIT_BMP_HEIGHT] PROGMEM = {
  {
    B01111000,
    B10000100,
    B10000100,
    B01111000,
    B00000000,
  },
  {
    B00000000,
    B00001000,
    B11111100,
    B00000000,
    B00000000,
  },
  {
    B10011000,
    B11000100,
    B10100100,
    B10011000,
    B00000000,
  },
  {
    B01000100,
    B10010100,
    B10010100,
    B01101000,
    B00000000,
  },
  {
    B01110000,
    B01001000,
    B11111100,
    B01000000,
    B00000000,
  },
  {
    B01011100,
    B10010100,
    B10010100,
    B01100100,
    B00000000,
  },
  {
    B01111000,
    B10010100,
    B10010100,
    B01100100,
    B00000000,
  },
  {
    B00000100,
    B11100100,
    B00010100,
    B00001100,
    B00000000,
  },
  {
    B01101000,
    B10010100,
    B10010100,
    B01101000,
    B00000000,
  },
  {
    B00001000,
    B10010100,
    B10010100,
    B01111000,
    B00000000,
  },
};


// 液晶ディスプレイ初期化
bool SSD1306_init(void) {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  return display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
}

// 数字を描画
void draw_number(int x, int y, unsigned int number, unsigned int digits_count) {
  for (int i = 0; i < digits_count; i++) {
    draw_digit(x, (digits_count - 1 - i) * DIGIT_BMP_HEIGHT + y, number % 10);
    number /= 10;
  }
}

// 1桁数字を描画
void draw_digit(int x, int y, unsigned int digit) {
  display.drawBitmap(x, y, digit_bmp[digit], DIGIT_BMP_WIDTH, DIGIT_BMP_HEIGHT, SSD1306_WHITE);
}
