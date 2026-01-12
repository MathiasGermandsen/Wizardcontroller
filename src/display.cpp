#include "display.h"
#include "config.h"

void initDisplay()
{
  carrier.display.setRotation(0);
}

void drawCentered(const char *text, int y, int size, uint16_t color)
{
  carrier.display.setTextSize(size);
  carrier.display.setTextColor(color);
  int textWidth = strlen(text) * 6 * size;
  int x = (SCREEN_WIDTH - textWidth) / 2;
  carrier.display.setCursor(x, y);
  carrier.display.print(text);
}

void drawCentered(String text, int y, int size, uint16_t color)
{
  drawCentered(text.c_str(), y, size, color);
}

void clearScreen(uint16_t color)
{
  carrier.display.fillScreen(color);
}

void drawProgressBar(int x, int y, int width, int height, int value, int maxValue, uint16_t fillColor)
{
  carrier.display.fillRect(x, y, width, height, ST77XX_BLACK);
  carrier.display.drawRect(x, y, width, height, ST77XX_WHITE);

  int fillWidth = 0;
  if (maxValue > 0)
  {
    fillWidth = (value * (width - 4)) / maxValue;
    if (fillWidth < 0)
      fillWidth = 0;
    if (fillWidth > width - 4)
      fillWidth = width - 4;
  }

  if (fillWidth > 0)
  {
    carrier.display.fillRect(x + 2, y + 2, fillWidth, height - 4, fillColor);
  }
}

uint16_t getHealthColor(int current, int max)
{
  if (max <= 0)
    return ST77XX_RED;

  int percent = (current * 100) / max;

  if (percent > 50)
  {
    return ST77XX_GREEN;
  }
  else if (percent > 25)
  {
    return ST77XX_YELLOW;
  }
  else
  {
    return ST77XX_RED;
  }
}

void showMessage(const char *line1, const char *line2, uint16_t bgColor)
{
  clearScreen(bgColor);
  drawCentered(line1, SCREEN_HEIGHT / 2 - 20, 2, ST77XX_WHITE);
  drawCentered(line2, SCREEN_HEIGHT / 2 + 10, 2, ST77XX_WHITE);
}

void drawPlayerStats(int hp, int maxHp, int mana, int maxMana)
{
  // HP bar øverst
  drawCentered("HP", 10, 1, ST77XX_WHITE);
  drawProgressBar(20, 25, SCREEN_WIDTH - 40, 20, hp, maxHp, getHealthColor(hp, maxHp));

  // HP tal
  String hpText = String(hp) + "/" + String(maxHp);
  drawCentered(hpText, 28, 1, ST77XX_WHITE);

  // Mana bar
  drawCentered("MANA", 55, 1, ST77XX_CYAN);
  drawProgressBar(20, 70, SCREEN_WIDTH - 40, 20, mana, maxMana, ST77XX_BLUE);

  // Mana tal
  String manaText = String(mana) + "/" + String(maxMana);
  drawCentered(manaText, 73, 1, ST77XX_WHITE);
}
