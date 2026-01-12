#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

enum class Button
{
  NONE = -1,
  TOUCH0 = 0,
  TOUCH1 = 1,
  TOUCH2 = 2,
  TOUCH3 = 3,
  TOUCH4 = 4
};

void updateInput();
bool isButtonPressed(int button);
bool isButtonPressed(Button button);
bool isButtonHeld(int button);
bool isButtonHeld(Button button);
Button getAnyButtonPressed();

extern MKRIoTCarrier carrier;

#endif
