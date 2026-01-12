#ifndef CONFIG_H
#define CONFIG_H

const char WIFI_SSID[] = "";
const char WIFI_PASS[] = "";

const char SERVER_IP[] = ""; // Opdater med serverens IP
const int SERVER_PORT = 3000;

const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 240;

const unsigned long HEARTBEAT_INTERVAL = 3000; // 3 sekunder mellem heartbeats

namespace Spells
{
  const char *const FIREBALL = "FIREBALL";       // 25 dmg, 20 mana
  const char *const LIGHTNING = "LIGHTNING";     // 15 dmg til ALLE, 30 mana
  const char *const SHIELD = "SHIELD";           // 50% dmg reduktion i 5 sek, 25 mana
  const char *const HEAL = "HEAL";               // Heal 20 HP, 35 mana
  const char *const POWER_BOOST = "POWER_BOOST"; // +50% dmg i 8 sek, 40 mana
  const char *const DEATH_RAY = "DEATH_RAY";     // 40 dmg, 50 mana
}

#endif
