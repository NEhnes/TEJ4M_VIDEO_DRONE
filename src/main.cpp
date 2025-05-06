/*
    BRANCH TEST TEST TEST BRANCH
*/

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include "esp_camera.h"

String speedValue = " ";

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // might not be needed
#endif

// Camera configuration for XIAO ESP32S3 Sense
#define PWDN_GPIO_NUM -1  // Power down is not used
#define RESET_GPIO_NUM -1 // Software reset
#define XCLK_GPIO_NUM 10
#define SIOD_GPIO_NUM 40
#define SIOC_GPIO_NUM 39
#define Y9_GPIO_NUM 48
#define Y8_GPIO_NUM 11
#define Y7_GPIO_NUM 12
#define Y6_GPIO_NUM 14
#define Y5_GPIO_NUM 16
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 17
#define Y2_GPIO_NUM 15
#define VSYNC_GPIO_NUM 38
#define HREF_GPIO_NUM 47
#define PCLK_GPIO_NUM 13

// login credentials
const char *ssid = "Tupperware";
const char *password = "meals4you";

const char *phone_ssid = "nathan_iPhone";
const char *phone_password = "nathan2024";

const char *school_ssid = "amdsb-guest";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int msecs, lastMsecs;
int frameInterval = 200;

void broadcastCameraFrame();

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  msecs = millis();
  lastMsecs = millis();

  // Camera config
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_VGA;    // 640x480
  config.pixel_format = PIXFORMAT_JPEG; // For streaming
  config.grab_mode = CAMERA_GRAB_LATEST;  //DEFAULT: GRAB_WHEN_EMPTY
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 50; // 10-63, lower number means higher quality   // DEFAULT: 12
  config.fb_count = 1; // DEFAULT: 1

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("camera init failed with error 0x%x", err);
    return;
  }
  else
  {
    Serial.println("camera init successful");
  }

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
      // recv and build msg string
      String msg = "";
      for (size_t i = 0; i < len; i++) {
        msg += (char)data[i];
      }

      //Serial.println("Received: " + msg);

      /* PARSE JOYSTICK DATA */
      if (msg.substring(0, 6) != "AT REST"){
        int speedStartIndex = msg.indexOf("SPEED: ") + 7;
        String speedString = msg.substring(speedStartIndex, speedStartIndex + 3); // grab first 3 char (incl '.')
        if (speedString.substring(2, 2) == "."){
          speedString.remove(2);
        }
        Serial.println("SPEED: ");
        Serial.println(speedString);
      }
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

  // add WebSocket
  server.addHandler(&ws);
}

void loop()
{
  msecs = millis();

  if (msecs - lastMsecs > frameInterval)
  {
    broadcastCameraFrame();
    lastMsecs = msecs;
  }

  if (msecs - lastMsecs > 5000) // print IP every 5s -> not wokring rn
  {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    lastMsecs = millis();
  }
  ws.cleanupClients();
}

void broadcastCameraFrame()
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }

  // === HIGHLIGHT: Skip sending if WebSocket buffer is full === //
  if (ws.availableForWriteAll()) {
    ws.binaryAll(fb->buf, fb->len);
  } else {
    Serial.println("Skipped frame: WebSocket buffer full");
  }

  // Return frame buffer
  esp_camera_fb_return(fb);
}