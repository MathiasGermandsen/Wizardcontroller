#include "input.h"

static bool previousStates[5] = {false, false, false, false, false};
static bool currentStates[5] = {false, false, false, false, false};

void updateInput()
{
  carrier.Buttons.update();

  for (int i = 0; i < 5; i++)
  {
    previousStates[i] = currentStates[i];
  }

  currentStates[0] = carrier.Buttons.onTouchDown(TOUCH0);
  currentStates[1] = carrier.Buttons.onTouchDown(TOUCH1);
  currentStates[2] = carrier.Buttons.onTouchDown(TOUCH2);
  currentStates[3] = carrier.Buttons.onTouchDown(TOUCH3);
  currentStates[4] = carrier.Buttons.onTouchDown(TOUCH4);
}

bool isButtonPressed(int button)
{
  if (button < 0 || button > 4)
    return false;
  return currentStates[button] && !previousStates[button];
}

bool isButtonPressed(Button button)
{
  if (button == Button::NONE)
    return false;
  return isButtonPressed(static_cast<int>(button));
}

bool isButtonHeld(int button)
{
  if (button < 0 || button > 4)
    return false;
  return currentStates[button];
}

bool isButtonHeld(Button button)
{
  if (button == Button::NONE)
    return false;
  return isButtonHeld(static_cast<int>(button));
}

Button getAnyButtonPressed()
{
  for (int i = 0; i < 5; i++)
  {
    if (isButtonPressed(i))
    {
      return static_cast<Button>(i);
    }
  }
  return Button::NONE;
}
