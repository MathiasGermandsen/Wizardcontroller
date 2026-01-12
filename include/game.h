#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include <Arduino_JSON.h>

enum class GameState
{
  IDLE,
  JOINING_QUEUE,
  IN_QUEUE,
  IN_GAME,
  GAME_OVER
};

struct PlayerStats
{
  int hp = 100;
  int maxHp = 100;
  int mana = 100;
  int maxMana = 100;
  bool hasShield = false;
  bool hasPowerBoost = false;
};

void initGame();
GameState getGameState();
void setGameState(GameState state);
bool joinQueue();
bool sendHeartbeat();
bool castSpell(const char *spellKey, String targetId = "");
void updateGame();
PlayerStats &getPlayerStats();
void parseGameState(String json);
bool shouldUpdateUI();
void clearUIUpdateFlag();
String getTargetPlayerId();
void setTargetPlayerId(String target);
void updateAvailableTargets(int *targets, int count);
int getAvailableTargetCount();
int *getAvailableTargets();
int getMyWizardId();
bool didPlayerWin();

#endif // GAME_H
