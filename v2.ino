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
#include "secrets.h"

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
      criticalScreen("Device cannot connectto your wifi network.", "Reprogram me or whitelist me in your\nrouter settings.\n\nMac Address:", developerTools);
    }
  }

  tft.println("Connected to Wifi!");
  tft.println("devmode: ");
  tft.print(developerTools);
  Serial.println("\nConnected to Wifi\n");
  delay(250);
  welcomeScreen(developerTools);

  if (MDNS.begin("picow")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleData);
  server.on("/welcome", []() {
    welcomeScreen(developerTools);  // Pass the developerTools argument
  });
  server.onNotFound(handleNotFound);
  server.on("/reboot", rebootSystem);
  server.on("/bugCheck", testError);
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

void beepInit() {
  if (beeperEnabled == true) {
    pinMode(22, OUTPUT);
  };
}

void beepFunction(bool enable) {
  if (beeperEnabled == true) {
    if (enable == true) {
      tone(22, 440);
    } else if (enable == false) {
      noTone(22);
    }
  }
}

String getData() {
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.print("Current time: ");

  tempC = get_temp_c();
  tempF = get_temp_f();
  pressure = get_pressure();

  // ---- Temperature Values ----
  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.println(" C");

  Serial.print("Temperature: ");
  Serial.print(tempF);
  Serial.println(" F");

  // ---- Pressure Values ----
  Serial.print("Measured Air Pressure: ");
  Serial.print(pressure, 2);
  Serial.println(" mb");
  Serial.println("\n");

  if (enableStatReadouts == true) {
    tft.fillScreen(BLACK);
    tft.setCursor(0, 0);
    tft.println("Successful request!");
    tft.print("Temperature: ");
    tft.print(tempC);
    tft.println(" C");

    tft.print("Temperature: ");
    tft.print(tempF);
    tft.println(" F");

    // ---- Pressure Values ----
    tft.print("Measured Air Pressure");
    tft.print(pressure, 2);
    tft.println(" mb\n");

    tft.println("Uptime as of this\nrequest:");
    tft.println(UptimeString::getUptime3());

    tft.println("\nAssigned IP Address:");
    tft.println(WiFi.localIP());
    previousInterval = currentInterval;
  }

  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off by making the voltage LOW

  // Fill JSON data
  data["sensor"] = "SPL06";
  data["tempC"] = tempC;
  data["tempF"] = tempF;
  data["airPressure"] = pressure;

  // Serialize to JSON
  serializeJsonPretty(data, exportedData);

  return exportedData;
}

void handleData() {
  String token = server.arg("token");
  if (token == serialID) {
    server.send(200, "application/json", getData() + "\r\n");
  } else {
    server.send(401, "text/plain", "Unauthorized access detected. Check the Diagnostic screen for more details and the token.");
    if (enableNonessentialErrorScreen == true) {
      tft.fillScreen(RED);
      tft.setCursor(0, 0);
      tft.println("!ACCESS DENIED!\n");
      tft.println("Invalid request\ndetected!\n");
      tft.println("Use this config for\nyour companion app:\n");
      tft.println(WiFi.localIP());
      tft.println("/?token=[token]\n");
      tft.println("The token is:");
      tft.println(serialID);
      previousInterval = currentInterval;
    }
  }
}


void rebootSystem() {
  String token = server.arg("token");
  if (token == serialID) {
    server.send(200, "text/plain", "Rebooting the system. Refer to the Diagnostic screen for more information.");
    *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
  } else {
    if (enableNonessentialErrorScreen == true) {
      server.send(401, "text/plain", "Unauthorized access detected. Check the Diagnostic screen for more details and the token.");
      tft.fillScreen(RED);
      tft.setCursor(0, 0);
      tft.println("!ACCESS DENIED!\n");
      tft.println("Invalid request\ndetected!\n");
      tft.println("Use this config for\nyour companion app:\n");
      tft.println(WiFi.localIP());
      tft.println("/reboot?token=[token]\n");
      tft.println("The token is:");
      tft.println(serialID);
      previousInterval = currentInterval;
    }
  }
}


void testError() {
  if (developerTools == true) {
    server.send(200, "text/plain", "Bug check made!");
    criticalScreen("Test bugcheck", "Test Bugcheck", true);
  }
  server.send(403, "text/plain", "Developer tools are off. Reenable developer tools in order to use this feature.");
}
