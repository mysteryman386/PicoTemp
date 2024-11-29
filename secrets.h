/////////CHANGABLE SETTINGS
// Screensaver timer (In milliseconds)
const unsigned long screensaverInterval = 600000;
const bool beeperEnabled = false;
const bool enableNonessentialErrorScreen = false; //You should disable this if you port forward in order to reduce OLED wear.
const bool enableStatReadouts = false; //This setting should also be set if you port forward your device. This setting reduces OLED
// WiFi Credentials
// Do be aware that the Raspberry Pi Pico W is only 2.4ghz capable.
#define WIFI_SSID "[CHANGE AS NEEDED]"
#define WIFI_PASS "[CHANGE AS NEEDED]"
// Developer tools (DO NOT ENABLE THIS IN PRODUCTION)
const bool developerTools = false;
/////////END OF CHANGABLE SETTINGS
