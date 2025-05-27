/*
    Camera stream working before current version. adding in new shi for motor driver
*/

#pragma region NEW_GLOBAL

#include <Arduino.h>

#define AIN1_PIN 3
#define AIN2_PIN 4
#define PWM_A 5
bool aForward = true;

#define BIN1_PIN 2
#define BIN2_PIN 1
#define PWM_B 0
bool bForward = true;

#define STBY_PIN 10

int lMotorSpeed = 50;
int rMotorSpeed = 50;

#pragma endregion

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

const char *phone_ssid = "nathan_iPhone";
const char *phone_password = "nathan2024";

const char *school_ssid = "amdsb-guest";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int msecs, lastMsecs;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(LED_BUILTIN, HIGH);

  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(BIN1_PIN, OUTPUT);
  pinMode(BIN2_PIN, OUTPUT);
  pinMode(STBY_PIN, OUTPUT);
  pinMode(PWM_A, OUTPUT);
  pinMode(PWM_B, OUTPUT);

  digitalWrite(STBY_PIN, HIGH);

  msecs = millis();
  lastMsecs = millis();

  Serial.begin(115200);
  Serial.println();
  Serial.println("Connecting...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
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
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
             {
    if (type == WS_EVT_DATA) {
      String msg = "";
      for (size_t i = 0; i < len; i++) {
        msg += (char)data[i];
      }
      Serial.println("Received: " + msg);

      if (msg.substring(0, 6) != "AT REST"){
        int speedStartIndex = msg.indexOf("SPEED: ") + 7;
        String speedString = msg.substring(speedStartIndex, speedStartIndex + 3); // grab first 3 char (incl '.')
        if (speedString.substring(2, 2) == "."){
          speedString.remove(2);
        }
        Serial.println("ONLY SPEED: ");
        Serial.println(speedString);
        lMotorSpeed = constrain(speedString.toInt(), 0, 100);
        rMotorSpeed = lMotorSpeed;
      }

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
    } });

  // Serve webpage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
      request->send(404, "text/plain", "File not found");
      return;
    }
    request->send(SPIFFS, "/index.html", "text/html");
    file.close(); });

  // Start server and WebSocket
  server.addHandler(&ws);
  server.begin();
  Serial.println("Server started");
}

void loop()
{
  // print ip every 5s
  msecs = millis();
  if (msecs - lastMsecs > 5000)
  {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    lastMsecs = millis();
  }
  ws.cleanupClients();

  // Serial.print("LEFT: ");
  // Serial.println(lMotorSpeed);
  // Serial.print("RIGHT: ");
  // Serial.println(rMotorSpeed);

}