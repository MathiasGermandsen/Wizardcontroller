#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFiNINA.h>

void connectWiFi();
String generateDeviceId();
bool isWiFiConnected();
String httpReadBody();
bool httpPost(const char *endpoint, String jsonBody, String &responseBody);

extern WiFiClient client;
extern String deviceId;

#endif
