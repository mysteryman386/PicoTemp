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
String exportedData;
byte mac[6];


Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

WebServer server(80);

JsonDocument data;
UptimeString uptimeString;



void criticalScreen(String lineOne, String lineTwo, bool devMessage) {
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
  if (devMessage == true) {
    tft.println("DevTools On\nReprogam to disable.\n");
  }
  delay(10000);
  *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
}

void welcomeScreen(bool devMessage) {
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
  if (devMessage == true) {
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

