#include <SPL06-007.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LEAmDNS.h>
#include <ArduinoJson.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <UptimeString.h>
#include <HTTPClient.h>

#include "essentialFunctions.h"

void setup() {
  tft.begin();
  lcdTestPattern();
  delay(500);
  tft.fillScreen(BLACK);
  Wire.begin();          // begin Wire(I2C)
  Serial.begin(115200);  // begin Serial
  SPL_init();            // Setup initial SPL chip registers
  watchdogIncrement = 0;
  serialID = rp2040.getChipID();
  beepInit();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    tft.print(".");
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
    watchdogIncrement = watchdogIncrement + 5;
    beepFunction(true);
    delay(150);
    beepFunction(false);
    delay(100);
    if (watchdogIncrement >= 300) {
      criticalScreen("Device cannot connectto your wifi network.", "Reprogram me or whitelist me in your\nrouter settings.\n\nMac Address:");
    }
  }

  tft.println("Connected to Wifi!");
  tft.println("devmode: ");
  tft.print(developerTools);
  Serial.println("\nConnected to Wifi\n");
  delay(250);
  welcomeScreen();

  if (MDNS.begin("picow")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleData);
  server.on("/welcome", []() {
    welcomeScreen();  
  });
  server.onNotFound(handleNotFound);
  server.on("/reboot", rebootSystem);
  server.on("/bugCheck", testError);
  server.on("/updatePressure", HTTP_POST, handlePostUpdatePressure);
  server.begin();
}


void loop() {
  server.handleClient();
  MDNS.update();

  currentInterval = millis();

  if (currentInterval - previousInterval >= screensaverInterval) {
    previousInterval = currentInterval;
    tft.fillScreen(BLACK);
    tft.setCursor(0, 0);
  }
}