#include "SSD1306_display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define OLED_RESET    -1  // sharing Arduino reset pin
#define SCREEN_ADDRESS  0x3C  // I2C address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


bool SSD1306_init(void) {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  return display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
}
