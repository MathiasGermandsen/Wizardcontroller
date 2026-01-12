#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include "config.h"
#include "network.h"
#include "display.h"
#include "input.h"
#include "game.h"

MKRIoTCarrier carrier;

// Selection enum for target handling
enum Selection
{
  NEXT,
  PREVIOUS
};

// Spell selection
const char *SPELL_LIST[] = {
    Spells::FIREBALL,
    Spells::LIGHTNING,
    Spells::SHIELD,
    Spells::HEAL,
    Spells::POWER_BOOST,
    Spells::DEATH_RAY};
const char *SPELL_NAMES[] = {"FIREBALL", "LIGHTNING", "SHIELD", "HEAL", "POWER BOOST", "DEATH RAY"};
const uint16_t SPELL_COLORS[] = {ST77XX_ORANGE, ST77XX_YELLOW, ST77XX_CYAN, ST77XX_GREEN, ST77XX_MAGENTA, ST77XX_RED};
const int SPELL_COUNT = 6;
int currentSpellIndex = 0;

// Target selection (0-3 wizard IDs)
int selectedTarget = 0;

// RGB colors for LED effects (matching spell colors)
// Format: {R, G, B}
const uint8_t SPELL_RGB[][3] = {
    {255, 165, 0}, // FIREBALL - Orange
    {255, 255, 0}, // LIGHTNING - Yellow
    {0, 255, 255}, // SHIELD - Cyan
    {0, 255, 0},   // HEAL - Green
    {255, 0, 255}, // POWER_BOOST - Magenta
    {255, 0, 0}    // DEATH_RAY - Red
};

void handleGameInput();
void drawGameUI();
void nextSpell();
void prevSpell();
void handleTargetSelection(Selection selection);
void flashLEDs(uint8_t r, uint8_t g, uint8_t b, int times, int delayMs);
void showDeadScreen();
void showWinScreen();

void setup()
{
  Serial.begin(9600);
  delay(2000);

  Serial.println("=================================");
  Serial.println("   WizardDuel Controller v1.0   ");
  Serial.println("=================================");

  carrier.noCase();
  if (!carrier.begin())
  {
    Serial.println("Errir: Kunne ikke initialisere carrier!");
    while (1)
      ;
  }
  Serial.println("Carrier initialiseret OK");

  initDisplay();
  showMessage("WizardDuel", "Starter...", ST77XX_BLUE);
  delay(1000);

  connectWiFi();
  deviceId = generateDeviceId();
  initGame();
  showMessage("Press to", "start!", ST77XX_GREEN);
}

void loop()
{
  updateInput();
  updateGame();

  GameState state = getGameState();

  switch (state)
  {
  case GameState::IDLE:
    if (getAnyButtonPressed() != Button::NONE)
    {
      joinQueue();
    }
    break;

  case GameState::IN_QUEUE:
    break;

  case GameState::IN_GAME:
  {
    // Check if dead
    PlayerStats &stats = getPlayerStats();
    if (stats.hp <= 0)
    {
      showDeadScreen();
      break;
    }

    // Initialize selectedTarget if it's our own wizard
    if (selectedTarget == getMyWizardId())
    {
      handleTargetSelection(NEXT);
    }
    handleGameInput();
    if (shouldUpdateUI())
    {
      drawGameUI();
      clearUIUpdateFlag();
    }
    break;
  }

  case GameState::GAME_OVER:
    if (didPlayerWin())
    {
      showWinScreen();
    }
    else
    {
      showDeadScreen();
    }

    // TOUCH2 (center) to exit and start new game
    if (isButtonPressed(Button::TOUCH2))
    {
      // Turn off LEDs
      for (int led = 0; led < 5; led++)
      {
        carrier.leds.setPixelColor(led, 0);
      }
      carrier.leds.show();

      initGame();
      showMessage("Press to", "start!", ST77XX_GREEN);
      delay(300);
    }
    break;

  default:
    break;
  }

  delay(50);
}

void handleGameInput()
{
  // TOUCH0 (left) - Previous spell
  if (isButtonPressed(Button::TOUCH0))
  {
    prevSpell();
    drawGameUI();
    delay(200);
  }
  // TOUCH4 (top) - Next spell
  else if (isButtonPressed(Button::TOUCH4))
  {
    nextSpell();
    drawGameUI();
    delay(200);
  }
  // TOUCH1 (bottom) - Previous target
  else if (isButtonPressed(Button::TOUCH1))
  {
    handleTargetSelection(PREVIOUS);
    drawGameUI();
    delay(200);
  }
  // TOUCH3 (right) - Next target
  else if (isButtonPressed(Button::TOUCH3))
  {
    handleTargetSelection(NEXT);
    drawGameUI();
    delay(200);
  }
  // TOUCH2 (center) - Cast spell!
  else if (isButtonPressed(Button::TOUCH2))
  {
    String targetId = String(selectedTarget);

    // Flash LEDs with spell color
    flashLEDs(SPELL_RGB[currentSpellIndex][0],
              SPELL_RGB[currentSpellIndex][1],
              SPELL_RGB[currentSpellIndex][2],
              2, 100);

    castSpell(SPELL_LIST[currentSpellIndex], targetId);
    showMessage(SPELL_NAMES[currentSpellIndex], "CAST!", SPELL_COLORS[currentSpellIndex]);
    delay(300);
    drawGameUI();
  }
}

void nextSpell()
{
  currentSpellIndex = (currentSpellIndex + 1) % SPELL_COUNT;
  Serial.print("Spell: ");
  Serial.println(SPELL_NAMES[currentSpellIndex]);
}

void prevSpell()
{
  currentSpellIndex = (currentSpellIndex - 1 + SPELL_COUNT) % SPELL_COUNT;
  Serial.print("Spell: ");
  Serial.println(SPELL_NAMES[currentSpellIndex]);
}

void handleTargetSelection(Selection selection)
{
  int wizardId = getMyWizardId();

  if (selection == NEXT)
  {
    while (1)
    {
      selectedTarget = (selectedTarget + 1) % 4;

      if (selectedTarget != wizardId)
      {
        break;
      }
    }
  }
  else if (selection == PREVIOUS)
  {
    while (1)
    {
      selectedTarget = (selectedTarget - 1);

      if (selectedTarget < 0)
      {
        selectedTarget = 3;
      }

      if (selectedTarget != wizardId)
      {
        break;
      }
    }
  }

  Serial.print("Target: Wizard ");
  Serial.println(selectedTarget);
}

void drawGameUI()
{
  clearScreen(ST77XX_BLACK);
  PlayerStats &stats = getPlayerStats();

  // HP bar at top
  drawCentered("HP", 5, 1, ST77XX_WHITE);
  drawProgressBar(20, 18, SCREEN_WIDTH - 40, 16, stats.hp, stats.maxHp, getHealthColor(stats.hp, stats.maxHp));
  String hpText = String(stats.hp) + "/" + String(stats.maxHp);
  drawCentered(hpText, 20, 1, ST77XX_WHITE);

  // Mana bar
  drawCentered("MANA", 40, 1, ST77XX_CYAN);
  drawProgressBar(20, 53, SCREEN_WIDTH - 40, 16, stats.mana, stats.maxMana, ST77XX_BLUE);
  String manaText = String(stats.mana) + "/" + String(stats.maxMana);
  drawCentered(manaText, 55, 1, ST77XX_WHITE);

  // Status effects
  if (stats.hasShield || stats.hasPowerBoost)
  {
    String statusText = "";
    if (stats.hasShield)
      statusText += "[SHIELD] ";
    if (stats.hasPowerBoost)
      statusText += "[BOOST]";
    drawCentered(statusText, 75, 1, ST77XX_YELLOW);
  }

  // Target selection (display as +1, so wizard 0 = "Wizard 1")
  drawCentered("TARGET:", 95, 1, ST77XX_WHITE);
  String targetText = "Wizard " + String(selectedTarget + 1);
  drawCentered(targetText, 115, 2, ST77XX_RED);

  // Current spell selection
  drawCentered("SPELL:", 145, 1, ST77XX_WHITE);
  drawCentered(SPELL_NAMES[currentSpellIndex], 162, 2, SPELL_COLORS[currentSpellIndex]);

  // Control hints at bottom
  drawCentered("<0 Spell 4>", SCREEN_HEIGHT - 35, 1, ST77XX_WHITE);
  drawCentered("v1 Target 3^", SCREEN_HEIGHT - 22, 1, ST77XX_WHITE);
  drawCentered("[2] CAST!", SCREEN_HEIGHT - 8, 1, ST77XX_GREEN);
}

void flashLEDs(uint8_t r, uint8_t g, uint8_t b, int times, int delayMs)
{
  for (int i = 0; i < times; i++)
  {
    // Turn on all 5 LEDs
    for (int led = 0; led < 5; led++)
    {
      carrier.leds.setPixelColor(led, carrier.leds.Color(r, g, b));
    }
    carrier.leds.show();
    delay(delayMs);

    // Turn off all LEDs
    for (int led = 0; led < 5; led++)
    {
      carrier.leds.setPixelColor(led, 0);
    }
    carrier.leds.show();
    delay(delayMs);
  }
}

void showDeadScreen()
{
  static unsigned long lastFlash = 0;
  static bool ledsOn = false;
  unsigned long now = millis();

  // Show DEAD message on screen
  clearScreen(ST77XX_BLACK);
  drawCentered("YOU ARE", SCREEN_HEIGHT / 2 - 30, 2, ST77XX_RED);
  drawCentered("DEAD", SCREEN_HEIGHT / 2 + 10, 3, ST77XX_RED);
  drawCentered("[2] New Game", SCREEN_HEIGHT - 30, 1, ST77XX_WHITE);

  // Flash LEDs red every 500ms
  if (now - lastFlash >= 500)
  {
    lastFlash = now;
    ledsOn = !ledsOn;

    for (int led = 0; led < 5; led++)
    {
      if (ledsOn)
      {
        carrier.leds.setPixelColor(led, carrier.leds.Color(255, 0, 0));
      }
      else
      {
        carrier.leds.setPixelColor(led, 0);
      }
    }
    carrier.leds.show();
  }
}

void showWinScreen()
{
  static unsigned long lastFlash = 0;
  static int colorIndex = 0;
  unsigned long now = millis();

  // Show WIN message on screen
  clearScreen(ST77XX_BLACK);
  drawCentered("VICTORY!", SCREEN_HEIGHT / 2 - 30, 3, ST77XX_GREEN);
  drawCentered("You won!", SCREEN_HEIGHT / 2 + 20, 2, ST77XX_YELLOW);
  drawCentered("[2] New Game", SCREEN_HEIGHT - 30, 1, ST77XX_WHITE);

  // Rainbow LED effect every 200ms
  if (now - lastFlash >= 200)
  {
    lastFlash = now;
    colorIndex = (colorIndex + 1) % 6;

    for (int led = 0; led < 5; led++)
    {
      int c = (colorIndex + led) % 6;
      carrier.leds.setPixelColor(led, carrier.leds.Color(
                                          SPELL_RGB[c][0], SPELL_RGB[c][1], SPELL_RGB[c][2]));
    }
    carrier.leds.show();
  }
}
