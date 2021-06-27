#include "input.h"
#include "SSD1306_display.h"
#include "tetris.h"


static unsigned long frame_count = 0;
static const unsigned int fps = 1000;


void setup() {
  Serial.begin(9600);
  input_init();
  if (!SSD1306_init()) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
}


void loop() {
  update_button_input();
  
  tetris(frame_count, fps);
  frame_count++;

  delay((unsigned long)(1000 / fps));
}
