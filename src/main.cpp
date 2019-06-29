/*
 LED strip animations
 WS2812B 144 LEDS/Meter IP65/IP67/IP68 (1 meter strip)
 Logic Level Converter (1 pc, may work w/o but good practice to use)
 ESP8266 - ESP8266 NodeMCU WiFi Module (1pc)
 */

#include <Arduino.h>

#define SERIAL_OUT Serial                                       // 

#define SETUP_STAT_NONE 0
#define SETUP_STAT_INIT 1
#define SETUP_STAT_LED_COMPLETE 2
#define SETUP_STAT_NET_PENDING 3
#define SETUP_STAT_COMPLETE 4
#define SETUP_STAT_STOPPED 5

#define RUN_ON (1 << 0)
#define RUN_OFF (1 << 1)
#define BRAKE_ON (1 << 2)
#define BRAKE_OFF (1 << 3)
#define LEFT_ON (1 << 4)
#define LEFT_OFF (1 << 5)
#define RIGHT_ON (1 << 6)
#define RIGHT_OFF (1 << 7)

#define ANIM_NOISE (1 << 0)
#define ANIM_FIRE (1 << 1)
#define ANIM_RAIN (1 << 2)
#define ANIM_JUGGLE (1 << 3)
#define ANIM_PULSE (1 << 4)
#define ANIM_CONFETTI (1 << 5)
#define ANIM_GLITTER (1 << 6)

void statusIndicator(const uint8_t stat, const uint8_t netStat = 3/* WL_CONNECTED */, const char* ipLoc = "", const char* info = "");

// ----------------- NeoPixelBus -----------------------------

// NeoPixelBufferCylon
// This example will move a Cylon Red Eye back and forth across the 
// the full collection of pixels on the strip. 
//
// This will demonstrate the use of the NeoVerticalSpriteSheet 
// 

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

// The actual image is contained in the data structure in one of the Cylon*.h files
// You will need to use the one that has the same color feature as your NeoPixelBus
// There are two provided, but you can create your own easily enough using 
// free versions of Paint.Net and the plugin 
#include "Pg3d.h"
typedef NeoGrbFeature MyPixelColorFeature;

const uint16_t PixelCount = 180;                            // WS2812 60 LEDs/meter, 3 meter strip
const uint16_t AnimCount = 2;                               // max number of animations that will run
RgbColor rbg = RgbColor(0, 0, 0);                           // color used for status indicator (1st LED)

byte led_flags = 0;                                         // flags for tracking LED states (separate ON/OFF flags for each state should exist to account for spurts of false-positive irregularities)

#include "led.cpp"

// ----------------- ESP8266 Server -----------------------------

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <FS.h>

#define NET_NEEDS_SETUP (1 << 1)                            // determines if the network setup has been performed
#define DNS_PORT 53                                         // the port used for DNS (AP Mode only)
#define SSID_FILE "/ssid.txt"                               // wifi credential storage size in SPIFFS
#define NET_RESTART 0                                       // flag that indicates the net services should perform a hard reset of the CPU
#define NET_SOFT_STOP 1                                     // flag that indicates the net services should perform a soft stop by closing net servers
#define NET_SOFT_RESTART 2                                  // flag that indicates the net services should perform a soft restart by closing net servers, disconnecting from WiFi, reset the net parameters and restart
#define NET_WIFI_CAHNNEL 5                                  // wifi channel to use (1-13, default is 1)
const char* domain = "motomoon.lighting";                   // when present, the webserver will be ran with SSID as an Access Point Webserver (AP Mode), blank SSID as the username to connect to an existing AP (Station Mode)
const char ssidDefault[WL_SSID_MAX_LENGTH] = "Motomoon";    // SSID to generated AP (AP Mode) or existing network SSID to connect to (Station Mode)
const char passDefault[WL_WPA_KEY_MAX_LENGTH] = "motomoon"; // password to generated AP (AP Mode) or existing network password (Station Mode)
volatile byte net_flags = 0;                                // flags for tracking web configuration state
const IPAddress ip(192, 168, 1, 1);                         // IP address to use when in AP mode (blank domain) 
DNSServer dns;                                              // the DNS server instance (AP mode only)
ESP8266WebServer http(80);                                  // the web server instance with port
struct wifi_t {
  char ssid[WL_SSID_MAX_LENGTH];                            // the SSID/username
  char pass[WL_WPA_KEY_MAX_LENGTH];                         // the SSID password
} wifi;

#include "net.cpp"

// ----------------- Program ------------------------ 

void loop() {
  ledLoop();
  netLoop();
}

// LED fills for visual indication of connection statuses
void statusIndicator(const uint8_t stat, const uint8_t netStat, const char* ipLoc, const char* info) {
  if (stat == SETUP_STAT_INIT) { // turn on on-board LED indicating startup is in progress
    pinMode(LED_BUILTIN, OUTPUT); // on-board LED
    digitalWrite(LED_BUILTIN, LOW); SERIAL_OUT.println("\n---> STARTING <---");
    return;
  } else if (stat == SETUP_STAT_COMPLETE) {
    strip.ClearTo(RgbColor(0, 0, 0), 0, 0);
    strip.Show();
    digitalWrite(LED_BUILTIN, HIGH); SERIAL_OUT.println("---> READY <---");
    return;
  } else if (stat == SETUP_STAT_STOPPED) {
    digitalWrite(LED_BUILTIN, LOW); SERIAL_OUT.println("\n---> STOPPED <---");
  }
  RgbColor rgb = RgbColor(0, 0, 0);
  if (stat == SETUP_STAT_LED_COMPLETE) { // dark slate gray
    rgb = RgbColor(18, 31, 31); SERIAL_OUT.println("LED setup complete");
  } else if (netStat == WL_CONNECTED && strlen(ipLoc)) {
    rgb = RgbColor(0, 255, 0); SERIAL_OUT.printf("Net setup complete. Web access available at IP: %s\n%s\n", ipLoc, info);
  } else if (stat != SETUP_STAT_LED_COMPLETE) {
    switch (netStat) {
      case WL_IDLE_STATUS: // blue
        rgb = RgbColor(0, 0, 255); SERIAL_OUT.println("WiFi Idle...");
        break;
      case WL_SCAN_COMPLETED: // indigo
        rgb = RgbColor(29, 0, 51); SERIAL_OUT.println("WiFi Scan completed...");
        break;
      case WL_NO_SSID_AVAIL: // dark orange
        rgb = RgbColor(255, 140, 0); SERIAL_OUT.printf("WiFi No SSID Available for: %s\n", wifi.ssid);
        break;
      case WL_CONNECTED: // green yellow
        rgb = RgbColor(173, 255, 47); SERIAL_OUT.printf("WiFi Connected to: %s\n", wifi.ssid); WiFi.printDiag(SERIAL_OUT);
        break;
      case WL_CONNECT_FAILED: // red
        rgb = RgbColor(255, 0, 0); SERIAL_OUT.printf("WiFi Connection Failed for SSID: %s\n", wifi.ssid);
        break;
      case WL_CONNECTION_LOST: // magenta
        rgb = RgbColor(202, 31, 123); SERIAL_OUT.printf("WiFi Connection lost for SSID: %s\n", wifi.ssid);
        break;
      case WL_DISCONNECTED: // yellow
        rgb = RgbColor(255, 255, 0); SERIAL_OUT.printf("WiFi Disconnected from: %s\n", wifi.ssid);
        break;
    }
  }
  strip.ClearTo(rgb, 0, 0);
  strip.Show();
}

// ----------------- General Setup ------------------------ 

void setup() {
  //system_update_cpu_freq(160); // 80 MHz or 160 MHz, default is 80 MHz
  SERIAL_OUT.begin(115200);
  statusIndicator((uint8_t) SETUP_STAT_INIT);
  delay(1000); // ESP8266 init delay
  
  ledSetup();
  netSetup();

  delay(1000); // give some time for the net to settle
  statusIndicator((uint8_t) SETUP_STAT_COMPLETE);
}
