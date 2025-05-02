/*
    BRANCH TEST TEST TEST BRANCH





  STA mode, joins existing network and opens webserver.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>

String speedValue = " ";

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // might not be needed
#endif

#define ledPin1 21
#define ledPin2 22
#define ledPin3 23


// login credentials
const char *ssid = "Tupperware";
const char *password = "meals4you"; 

const char *school_ssid = "amdsb-guest";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Connecting...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(".");
  }
  
  // Print IP address
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
  Serial.println("Server started");

  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  // WebSocket event handler
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
      String msg = "";
      for (size_t i = 0; i < len; i++) {
        msg += (char)data[i];
      }
      Serial.println("Received: " + msg);

      // Parse joystick data (e.g., "x:50,y:30")
      // int x = 0, y = 0;
      // double joystickSpeed;
      // double joystickAngle;
      // if (msg.indexOf("SPEED: ") != -1 && msg.indexOf("y:") != -1) {
      //   x = msg.substring(msg.indexOf("x:") + 2, msg.indexOf(",")).toInt();
      //   y = msg.substring(msg.indexOf("y:") + 2).toInt();
      // }

      // Control logic (example: differential drive for motors or LEDs)
      // int leftSpeed = constrain(y + x, -100, 100); // Y (forward/back) + X (turn)
      // int rightSpeed = constrain(y - x, -100, 100);
    }
  });

  // Serve webpage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
      request->send(404, "text/plain", "File not found");
      return;
    }
    request->send(SPIFFS, "/index.html", "text/html");
    file.close();
  });

  // Start server and WebSocket
  server.addHandler(&ws);
  server.begin();
  Serial.println("Server started");
}

//complete up to here


void loop()
{
 ws.cleanupClients();
}