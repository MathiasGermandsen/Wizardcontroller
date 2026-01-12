#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

// Custom colors
#define ST77XX_GRAY 0x7BEF

void initDisplay();
void drawCentered(const char *text, int y, int size, uint16_t color);
void drawCentered(String text, int y, int size, uint16_t color);
void clearScreen(uint16_t color);
void drawProgressBar(int x, int y, int width, int height, int value, int maxValue, uint16_t fillColor);
uint16_t getHealthColor(int current, int max);
void showMessage(const char *line1, const char *line2, uint16_t bgColor);
void drawPlayerStats(int hp, int maxHp, int mana, int maxMana);

// Ekstern reference til carrier (defineret i main.cpp)
extern MKRIoTCarrier carrier;

#endif // DISPLAY_H
