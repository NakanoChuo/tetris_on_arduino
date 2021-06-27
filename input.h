#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>


#define BUTTON_UP      B0001
#define BUTTON_DOWN    B0010
#define BUTTON_LEFT    B0100
#define BUTTON_RIGHT   B1000


void input_init(void);
void update_button_input(void);
unsigned int read_button_input(void);
bool button_press(unsigned int button_code);
bool button_down(unsigned int button_code);
bool button_up(unsigned int button_code);


#endif
