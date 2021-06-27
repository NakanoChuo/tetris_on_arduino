#include "input.h"
#include <Arduino.h>


#define UP_BUTTON_PIN     2
#define DOWN_BUTTON_PIN   3
#define LEFT_BUTTON_PIN   4
#define RIGHT_BUTTON_PIN  5


static unsigned int input;
static unsigned int prev_input;
static unsigned int rising_edge_input;
static unsigned int falling_edge_input;
static unsigned int hold_input;


void input_init(void) {
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
}


void update_button_input(void) {
    input = read_button_input();
    rising_edge_input = input & ~prev_input;
    falling_edge_input = ~input & prev_input;
    prev_input = input;
}


unsigned int read_button_input(void) {
  return 
    ((digitalRead(UP_BUTTON_PIN) == LOW) * BUTTON_UP)
    | ((digitalRead(DOWN_BUTTON_PIN) == LOW) * BUTTON_DOWN)
    | ((digitalRead(LEFT_BUTTON_PIN) == LOW) * BUTTON_LEFT)
    | ((digitalRead(RIGHT_BUTTON_PIN) == LOW) * BUTTON_RIGHT);
}


bool button_press(unsigned int button_code) {
  return input & button_code;
}

bool button_down(unsigned int button_code) {
  return rising_edge_input & button_code;
}

bool button_up(unsigned int button_code) {
  return falling_edge_input & button_code;
}
