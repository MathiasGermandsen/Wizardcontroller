#include "game.h"
#include "config.h"
#include "network.h"
#include "display.h"

static GameState currentState = GameState::IDLE;
static PlayerStats playerStats;
static unsigned long lastHeartbeat = 0;
static String currentMatchId = "";
static String targetPlayerId = "";
static bool uiNeedsUpdate = false;
static int myWizardId = -1;
static int availableTargetsList[8];
static int availableTargetsCount = 0;
static bool playerWon = false;

void initGame()
{
  currentState = GameState::IDLE;
  playerStats = PlayerStats();
  lastHeartbeat = 0;
  currentMatchId = "";
  targetPlayerId = "";
  myWizardId = -1;
  availableTargetsCount = 0;
  playerWon = false;
  Serial.println("Game initialized");
}

GameState getGameState()
{
  return currentState;
}

void setGameState(GameState state)
{
  currentState = state;
  Serial.print("Game state changed to: ");
  switch (state)
  {
  case GameState::IDLE:
    Serial.println("IDLE");
    break;
  case GameState::JOINING_QUEUE:
    Serial.println("JOINING_QUEUE");
    break;
  case GameState::IN_QUEUE:
    Serial.println("IN_QUEUE");
    break;
  case GameState::IN_GAME:
    Serial.println("IN_GAME");
    break;
  case GameState::GAME_OVER:
    Serial.println("GAME_OVER");
    break;
  }
}

bool joinQueue()
{
  setGameState(GameState::JOINING_QUEUE);
  showMessage("Joining", "queue...", ST77XX_BLUE);

  String jsonBody = "{\"deviceId\":\"" + deviceId + "\"}";
  String response;

  if (httpPost("/join-queue", jsonBody, response))
  {
    Serial.println("Join queue response: " + response);
    JSONVar parsed = JSON.parse(response);

    if (JSON.typeof(parsed) != "undefined")
    {
      // Check for "success" and "inQueue" instead of "status"
      if (parsed.hasOwnProperty("success") && parsed.hasOwnProperty("inQueue"))
      {
        bool success = (bool)parsed["success"];
        bool inQueue = (bool)parsed["inQueue"];

        if (success && inQueue)
        {
          setGameState(GameState::IN_QUEUE);
          showMessage("In queue!", "Waiting...", ST77XX_GREEN);
          return true;
        }
      }
    }
  }

  setGameState(GameState::IDLE);
  showMessage("Error!", "Try again", ST77XX_RED);
  delay(2000);
  return false;
}

bool sendHeartbeat()
{
  String jsonBody = "{\"deviceId\":\"" + deviceId + "\"}";
  String response;

  if (httpPost("/heartbeat", jsonBody, response))
  {
    Serial.println("Heartbeat response: " + response);
    JSONVar parsed = JSON.parse(response);

    if (JSON.typeof(parsed) != "undefined")
    {
      // Store old values to detect changes
      int oldHp = playerStats.hp;
      int oldMana = playerStats.mana;
      bool oldShield = playerStats.hasShield;
      bool oldBoost = playerStats.hasPowerBoost;

      // Parse stats regardless of status field
      if (parsed.hasOwnProperty("hp"))
      {
        playerStats.hp = (int)parsed["hp"];
      }
      if (parsed.hasOwnProperty("maxHp"))
      {
        playerStats.maxHp = (int)parsed["maxHp"];
      }
      if (parsed.hasOwnProperty("mana"))
      {
        playerStats.mana = (int)parsed["mana"];
      }
      if (parsed.hasOwnProperty("maxMana"))
      {
        playerStats.maxMana = (int)parsed["maxMana"];
      }
      if (parsed.hasOwnProperty("shield"))
      {
        playerStats.hasShield = (bool)parsed["shield"];
      }
      if (parsed.hasOwnProperty("boost"))
      {
        playerStats.hasPowerBoost = (bool)parsed["boost"];
      }

      // Check if stats changed - if so, mark UI for update
      if (oldHp != playerStats.hp || oldMana != playerStats.mana ||
          oldShield != playerStats.hasShield || oldBoost != playerStats.hasPowerBoost)
      {
        uiNeedsUpdate = true;
        Serial.println("Stats changed, UI update needed");
      }

      // Parse our wizard ID
      if (parsed.hasOwnProperty("wizardId"))
      {
        myWizardId = (int)parsed["wizardId"];
      }

      // Parse available targets (other wizards in the game)
      if (parsed.hasOwnProperty("players"))
      {
        JSONVar players = parsed["players"];
        int newCount = 0;
        for (int i = 0; i < players.length() && newCount < 8; i++)
        {
          int wizId = (int)players[i]["wizardId"];
          bool alive = true;
          if (players[i].hasOwnProperty("alive"))
          {
            alive = (bool)players[i]["alive"];
          }
          // Only add other alive wizards as targets
          if (wizId != myWizardId && alive)
          {
            availableTargetsList[newCount] = wizId;
            newCount++;
          }
        }
        if (newCount != availableTargetsCount)
        {
          availableTargetsCount = newCount;
          uiNeedsUpdate = true;
        }
      }

      // Check for state transition - server sends "inGame": true (boolean)
      if (parsed.hasOwnProperty("inGame"))
      {
        bool inGame = (bool)parsed["inGame"];

        if (inGame && currentState == GameState::IN_QUEUE)
        {
          if (parsed.hasOwnProperty("matchId"))
          {
            currentMatchId = (const char *)parsed["matchId"];
          }
          if (parsed.hasOwnProperty("targetId"))
          {
            targetPlayerId = (const char *)parsed["targetId"];
          }
          setGameState(GameState::IN_GAME);
          uiNeedsUpdate = true;
          Serial.println(">>> Transitioning to IN_GAME, screen should update");
        }
      }
    }
    return true;
  }
  return false;
}

bool shouldUpdateUI()
{
  return uiNeedsUpdate;
}

void clearUIUpdateFlag()
{
  uiNeedsUpdate = false;
}

String getTargetPlayerId()
{
  return targetPlayerId;
}

void setTargetPlayerId(String target)
{
  targetPlayerId = target;
  Serial.print("Target set to: ");
  Serial.println(target);
}

bool castSpell(const char *spellKey, String targetId)
{
  if (currentState != GameState::IN_GAME)
  {
    Serial.println("Cannot cast spell - not in game");
    return false;
  }

  if (targetId.length() == 0)
  {
    targetId = targetPlayerId;
  }

  String jsonBody = "{";
  jsonBody += "\"deviceId\":\"" + deviceId + "\",";
  jsonBody += "\"spellKey\":\"" + String(spellKey) + "\",";
  jsonBody += "\"targetId\":\"" + targetId + "\"";
  jsonBody += "}";

  String response;
  Serial.print("Casting spell: ");
  Serial.println(spellKey);

  if (httpPost("/cast-spell", jsonBody, response))
  {
    Serial.println("Spell cast response: " + response);
    JSONVar parsed = JSON.parse(response);

    if (JSON.typeof(parsed) != "undefined")
    {
      if (parsed.hasOwnProperty("hp"))
      {
        playerStats.hp = (int)parsed["hp"];
      }
      if (parsed.hasOwnProperty("mana"))
      {
        playerStats.mana = (int)parsed["mana"];
      }

      if (parsed.hasOwnProperty("gameOver"))
      {
        bool gameOver = (bool)parsed["gameOver"];
        if (gameOver)
        {
          setGameState(GameState::GAME_OVER);

          playerWon = false;
          if (parsed.hasOwnProperty("winner"))
          {
            String winner = (const char *)parsed["winner"];
            playerWon = (winner == deviceId);
          }

          // Don't show message here, let main.cpp handle the screens
        }
      }
      return true;
    }
  }
  return false;
}

void updateGame()
{
  unsigned long now = millis();
  if (currentState == GameState::IN_QUEUE || currentState == GameState::IN_GAME)
  {
    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL)
    {
      sendHeartbeat();
      lastHeartbeat = now;
    }
  }
}

PlayerStats &getPlayerStats()
{
  return playerStats;
}

void parseGameState(String json)
{
  JSONVar parsed = JSON.parse(json);
  if (JSON.typeof(parsed) == "undefined")
  {
    Serial.println("Kunne ikke parse game state JSON");
    return;
  }

  if (parsed.hasOwnProperty("hp"))
    playerStats.hp = (int)parsed["hp"];
  if (parsed.hasOwnProperty("maxHp"))
    playerStats.maxHp = (int)parsed["maxHp"];
  if (parsed.hasOwnProperty("mana"))
    playerStats.mana = (int)parsed["mana"];
  if (parsed.hasOwnProperty("maxMana"))
    playerStats.maxMana = (int)parsed["maxMana"];
  if (parsed.hasOwnProperty("hasShield"))
    playerStats.hasShield = (bool)parsed["hasShield"];
  if (parsed.hasOwnProperty("hasPowerBoost"))
    playerStats.hasPowerBoost = (bool)parsed["hasPowerBoost"];
}

int getAvailableTargetCount()
{
  return availableTargetsCount;
}

int *getAvailableTargets()
{
  return availableTargetsList;
}

int getMyWizardId()
{
  return myWizardId;
}

bool didPlayerWin()
{
  return playerWon;
}
