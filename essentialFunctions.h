#include <SPL06-007.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LEAmDNS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <UptimeString.h>
#include <HTTPClient.h>
#include "secrets.h"


//Screensaver to prevent OLED burn in
unsigned long previousInterval = 0;
unsigned long currentInterval;

#define SCLK_PIN 18
#define MOSI_PIN 19
#define DC_PIN 20
#define CS_PIN 17
#define RST_PIN 21

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128  // Change this to 96 for 1.27" OLED.

String serialID;
int watchdogIncrement;
float tempC;
float tempF;
float pressure;
float altitude;
float altitudeF;
String exportedData;
byte mac[6];

unsigned long screensaverInterval = defaultScreensaverInterval;
bool beeperEnabled = defaultbeeperEnabled;
bool enableNonessentialErrorScreen = defaultenableNonessentialErrorScreen;
bool enableStatReadouts = defaultenableStatReadouts;
double localPressure = defaultlocalPressure;



Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

WebServer server(80);

JsonDocument data;
UptimeString uptimeString;



void criticalScreen(String lineOne, String lineTwo) {
  tft.fillScreen(RED);
  tft.setCursor(0, 0);
  tft.println("ERROR!\n");
  tft.println(lineOne);
  tft.println("Rebooting...\n");
  tft.println(lineTwo);
  WiFi.macAddress(mac);
  tft.print(mac[0], HEX);
  tft.print(":");
  tft.print(mac[1], HEX);
  tft.print(":");
  tft.print(mac[2], HEX);
  tft.print(":");
  tft.print(mac[3], HEX);
  tft.print(":");
  tft.print(mac[4], HEX);
  tft.print(":");
  tft.println(mac[5], HEX);
  tft.println("Serial Number:");
  tft.println(serialID);
  if (developerTools == true) {
    tft.println("DevTools On\nReprogam to disable.\n");
  }
  delay(10000);
  *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
}

void informationalScreen(String lineOne, String lineTwo) {
  tft.fillScreen(YELLOW);
  tft.setCursor(0, 0);
  tft.println("INFO PROMPT!\n");
  tft.println(lineOne);
  tft.println("INFO PROMPT!\n");
  tft.println(lineTwo);
  WiFi.macAddress(mac);
  tft.print(mac[0], HEX);
  tft.print(":");
  tft.print(mac[1], HEX);
  tft.print(":");
  tft.print(mac[2], HEX);
  tft.print(":");
  tft.print(mac[3], HEX);
  tft.print(":");
  tft.print(mac[4], HEX);
  tft.print(":");
  tft.println(mac[5], HEX);
  tft.println("Serial Number:");
  tft.println(serialID);
  if (developerTools == true) {
    tft.println("DevTools On\nReprogam to disable.\n");
  }
  delay(10000);
  *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
}

void welcomeScreen() {
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.print("Assigned IP Address:\n");
  tft.println(WiFi.localIP());
  tft.println("Connect your device\nto the companion app.\n");
  tft.println("Secret Token:");
  tft.println(serialID);
  Serial.println("\nIP Address assigned to device:");
  Serial.println(WiFi.localIP());
  WiFi.macAddress(mac);
  tft.println("Mac Address:");
  tft.print(mac[0], HEX);
  tft.print(":");
  tft.print(mac[1], HEX);
  tft.print(":");
  tft.print(mac[2], HEX);
  tft.print(":");
  tft.print(mac[3], HEX);
  tft.print(":");
  tft.print(mac[4], HEX);
  tft.print(":");
  tft.println(mac[5], HEX);
  if (developerTools == true) {
    tft.println("\nDevTools On\nReprogam to disable.\n");
  }
  previousInterval = currentInterval;
}


void lcdTestPattern() {
  static const uint16_t PROGMEM colors[] = { RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, BLACK, WHITE };

  for (uint8_t c = 0; c < 8; c++) {
    tft.fillRect(0, tft.height() * c / 8, tft.width(), tft.height() / 8, pgm_read_word(&colors[c]));
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "Error 404. Webpage not found.");
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
  altitude = get_altitude(get_pressure(),localPressure);
  altitudeF = get_altitude_f(get_pressure(),localPressure);

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
  data["localAirPressure"] = localPressure;
  data["altitudeM"] = altitude;
  data["altitudeF"] = altitudeF;

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
    criticalScreen("Test bugcheck", "Test Bugcheck");
  }
  server.send(403, "text/plain", "Developer tools are off. Reenable developer tools in order to use this feature.");
}

#include <ArduinoJson.h> // For parsing JSON

void handlePostUpdatePressure() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  // Check if the request has a body
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Bad Request: Missing Body");
    return;
  }

  // Parse JSON from the body
  String body = server.arg("plain");
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  // Check for token
  if (!doc.containsKey("token") || doc["token"].as<String>() != serialID) {
    server.send(403, "application/json", "{\"error\":\"Forbidden: Invalid token\"}");
    return;
  }

  // Check for pressure value
  if (!doc.containsKey("pressure")) {
    server.send(400, "application/json", "{\"error\":\"Missing 'pressure' field\"}");
    return;
  }

  double new_pressure = doc["pressure"].as<double>();

  // Update the local_pressure value
  localPressure = new_pressure;

  // Respond with success
  server.send(200, "application/json", "{\"message\":\"Pressure updated successfully\",\"localPressure\":" + String(localPressure, 2) + "}");
}
