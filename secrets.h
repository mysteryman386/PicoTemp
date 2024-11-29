/////////CHANGABLE SETTINGS
#define defaultScreensaverInterval 600000 // Screensaver timer (In milliseconds)
#define defaultbeeperEnabled false // Beep or no beep?
#define defaultenableNonessentialErrorScreen false //You should disable this if you port forward in order to reduce OLED wear.
#define defaultenableStatReadouts false //This setting should also be set if you port forward your device. This setting reduces OLED
#define defaultlocalPressure 1024.96
// WiFi Credentials
// Do be aware that the Raspberry Pi Pico W is only 2.4ghz capable.
#define WIFI_SSID "[PLACEHOLDER]"
#define WIFI_PASS "[PLACEHOLDER]" //This can be set as NULL if you are connecting to an open wifi network.
// Developer tools (DO NOT ENABLE THIS IN PRODUCTION)
const bool developerTools = false;
/////////END OF CHANGABLE SETTINGS
