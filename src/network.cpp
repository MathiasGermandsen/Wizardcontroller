#include "network.h"
#include "config.h"
#include "display.h"

WiFiClient client;
String deviceId = "";

void connectWiFi()
{
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  showMessage("Connecting to", "WiFi...", ST77XX_BLUE);

  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED)
  {
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);
    if (status != WL_CONNECTED)
    {
      Serial.println("Connection failed, retrying...");
      delay(2000);
    }
  }

  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  showMessage("WiFi OK!", WiFi.localIP().toString().c_str(), ST77XX_GREEN);
  delay(1000);
}

String generateDeviceId()
{
  byte mac[6];
  WiFi.macAddress(mac);

  String id = "";
  for (int i = 5; i >= 0; i--)
  {
    if (mac[i] < 0x10)
    {
      id += "0";
    }
    id += String(mac[i], HEX);
  }
  id.toUpperCase();

  Serial.print("Device ID: ");
  Serial.println(id);

  return id;
}

bool isWiFiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

String httpReadBody()
{
  String body = "";
  bool headerEnded = false;
  String line = "";

  unsigned long timeout = millis() + 5000; // 5 sek timeout

  while (client.connected() && millis() < timeout)
  {
    if (client.available())
    {
      char c = client.read();

      if (!headerEnded)
      {
        line += c;
        if (line.endsWith("\r\n\r\n"))
        {
          headerEnded = true;
        }
      }
      else
      {
        body += c;
      }
    }
  }

  return body;
}

bool httpPost(const char *endpoint, String jsonBody, String &responseBody)
{
  // Tjek WiFi forbindelse
  if (!isWiFiConnected())
  {
    Serial.println("WiFi Not Connected!");
    return false;
  }

  // Opret forbindelse til server
  if (!client.connect(SERVER_IP, SERVER_PORT))
  {
    Serial.println("Could'nt connect to WIFI");
    return false;
  }

  // Send HTTP request
  client.print("POST ");
  client.print(endpoint);
  client.println(" HTTP/1.1");

  client.print("Host: ");
  client.print(SERVER_IP);
  client.print(":");
  client.println(SERVER_PORT);

  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonBody.length());
  client.println("Connection: close");
  client.println();
  client.print(jsonBody);

  responseBody = httpReadBody();
  client.stop();

  if (responseBody.length() == 0)
  {
    Serial.println("No response");
    return false;
  }

  return true;
}
