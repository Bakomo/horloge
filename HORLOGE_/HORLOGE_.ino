/*
 * A program to demonstrate the use of SystemClockLoop. It should print the
 * following on the SERIAL_PORT_MONITOR port every 2 seconds:
 *
 *   2019-06-17T19:50:00-07:00[America/Los_Angeles]
 *   2019-06-17T19:50:02-07:00[America/Los_Angeles]
 *   2019-06-17T19:50:04-07:00[America/Los_Angeles]
 *   ...
 */

#include <Arduino.h>
#include <Array.h>

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Include the RTC library
#include "RTC.h"

//Include the NTP library
#include <NTPClient.h>

#if defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#endif

#include <WiFiUdp.h>
#include <FastLED.h>

char ssid[] = "Livebox-C208";        // your network SSID (name)
char pass[] = "wMKZ73pccQH6Gnrq7Y";    // your network password (use for WPA, or use as key for WEP)

int wifiStatus = WL_IDLE_STATUS;
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);

// ESP32 does not define SERIAL_PORT_MONITOR
#ifndef SERIAL_PORT_MONITOR
#define SERIAL_PORT_MONITOR Serial
#endif

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define DATA_PIN 6
#define NUM_PIXELS 120
#define BRIGHTNESS  64

#define NORMAL_MODE 0
#define CLOCK_SETTINGS 1
#define COLOR_SETTINGS 2

#define HOURS_SETTINGS 0
#define LARGE_MINUTES_SETTINGS 1
#define MINUTES_SETTINGS 2

// SETUP
int timeRefresh = 20000;
bool initialisation = true;

bool bright = true;
int brightDuring = 30;
int brightness = 30;

// LEDS
int clockColors[] = { 255, 255, 255};
typedef Array<int,NUM_PIXELS> Elements;
CRGB leds[NUM_PIXELS];
Elements ledsToShow;

// MODES DISPONIBLES
bool randomColorMode = false;
bool hardRandomColorMode = false;

int slideIndex = 1;
int countBrightness = 0;

int initHour = 0;
int initMinute = 0;
int initMinMinute = 0;

//-----------------------------------------------------------------------------

// BUTTON
int buttonPrincipalPin = 2;
int buttonUpPin = 3;
int buttonDownPin = 4;

bool buttonPrincipalPressed = false;
bool buttonUpPressed = false;
bool buttonDownPressed = false;

int modeSettings = NORMAL_MODE;
int clockSet = 0;

// int hoursAdded  = 0;
// int minutesAdded = 0;

auto timeZoneOffsetHours = 1;

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void connectToWiFi(){
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (wifiStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    wifiStatus = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  printWifiStatus();
}

void setup() {

#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  // SERIAL_PORT_MONITOR.begin(9600);
  while (!SERIAL_PORT_MONITOR);

  pinMode(buttonPrincipalPin, INPUT);
  digitalWrite(buttonPrincipalPin, HIGH);

  pinMode(buttonUpPin, INPUT);
  digitalWrite(buttonUpPin, HIGH);

  pinMode(buttonDownPin, INPUT);
  digitalWrite(buttonDownPin, HIGH);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_PIXELS);  // GRB ordering is typical
  FastLED.setBrightness(  BRIGHTNESS );
  for (int i = 0; i < 110; i++) {
    leds[i] = CRGB::White;
  }

  FastLED.show();

  // Serial.begin(9600);
  // while (!Serial);

  connectToWiFi();
  RTC.begin();
  // Serial.println("\nStarting connection to server...");
  timeClient.begin();
  timeClient.update();
  timeClient.setTimeOffset(timeZoneOffsetHours * 3600);

  auto unixTime = timeClient.getEpochTime();
  // Serial.print("Unix time = ");
  // Serial.println(unixTime);
  RTCTime timeToSet = RTCTime(unixTime);
  RTC.setTime(timeToSet);

  RTCTime currentTime;
  RTC.getTime(currentTime); 

}

void hoursPixels(int hours, bool plusHours) {
  if (plusHours) hours++;

  switch (hours) {
    case 0: // douze
    case 12: // douze
      ledsToShow.push_back(62);
      ledsToShow.push_back(77);
      ledsToShow.push_back(82);
      ledsToShow.push_back(95);
      ledsToShow.push_back(100);
      break;
    case 1: // une
      ledsToShow.push_back(79);
      ledsToShow.push_back(80);
      ledsToShow.push_back(97);
      break;
    case 2: // deux
      ledsToShow.push_back(3);
      ledsToShow.push_back(20);
      ledsToShow.push_back(23);
      ledsToShow.push_back(39);
      break;
    case 3: // trois
      ledsToShow.push_back(61);
      ledsToShow.push_back(78);
      ledsToShow.push_back(81);
      ledsToShow.push_back(96);
      ledsToShow.push_back(99);
      break;
    case 4: // quatre
      ledsToShow.push_back(4);
      ledsToShow.push_back(19);
      ledsToShow.push_back(24);
      ledsToShow.push_back(38);
      ledsToShow.push_back(43);
      ledsToShow.push_back(58);
      break;
    case 5: // cinq
      ledsToShow.push_back(5);
      ledsToShow.push_back(18);
      ledsToShow.push_back(25);
      ledsToShow.push_back(37);
      break;
    case 6: // six
      ledsToShow.push_back(44);
      ledsToShow.push_back(57);
      ledsToShow.push_back(63);
      break;
    case 7: // sept
      ledsToShow.push_back(76);
      ledsToShow.push_back(83);
      ledsToShow.push_back(94);
      ledsToShow.push_back(101);
      break;
    case 8: // huit
      ledsToShow.push_back(6);
      ledsToShow.push_back(17);
      ledsToShow.push_back(26);
      ledsToShow.push_back(36);
      break;
    case 9: // neuf
      ledsToShow.push_back(45);
      ledsToShow.push_back(56);
      ledsToShow.push_back(64);
      ledsToShow.push_back(75);
      break;
    case 10: // dix 
      ledsToShow.push_back(84);
      ledsToShow.push_back(93);
      ledsToShow.push_back(102);
      break;
    case 11: // onze
      ledsToShow.push_back(7);
      ledsToShow.push_back(16);
      ledsToShow.push_back(27);
      ledsToShow.push_back(35);
      break;
    default:
      SERIAL_PORT_MONITOR.print("Nothings");
      break;
  }
}

void largeMinutesPixels(int minutes) {

  if (minutes > 34) { // Moins

      ledsToShow.push_back(8);
      ledsToShow.push_back(15);
      ledsToShow.push_back(28);
      ledsToShow.push_back(34);
      ledsToShow.push_back(47);

  }

  if (minutes >= 55 || (minutes >= 5 && minutes < 10)) { // cinq

    ledsToShow.push_back(68);
    ledsToShow.push_back(71);
    ledsToShow.push_back(88);
    ledsToShow.push_back(89);

  } else if (minutes >= 50 || (minutes >= 10 && minutes < 15)) { // dix

    ledsToShow.push_back(86);
    ledsToShow.push_back(91);
    ledsToShow.push_back(104);

  } else if (minutes >= 45)  { // le quart

    ledsToShow.push_back(66);
    ledsToShow.push_back(73);

    ledsToShow.push_back(33);
    ledsToShow.push_back(48);
    ledsToShow.push_back(53);
    ledsToShow.push_back(67);
    ledsToShow.push_back(72);

  } else if (minutes >= 40 || (minutes >= 20 && minutes < 25)) { // vingt

    ledsToShow.push_back(10);
    ledsToShow.push_back(13);
    ledsToShow.push_back(30);
    ledsToShow.push_back(32);
    ledsToShow.push_back(49);

  } else if (minutes >= 35 || (minutes >= 25 && minutes < 30)) { // vingt-cinq

    ledsToShow.push_back(10);
    ledsToShow.push_back(13);
    ledsToShow.push_back(30);
    ledsToShow.push_back(32);
    ledsToShow.push_back(49);

    ledsToShow.push_back(52);

    ledsToShow.push_back(68);
    ledsToShow.push_back(71);
    ledsToShow.push_back(88);
    ledsToShow.push_back(89);

  } else if (minutes >= 30) { // et demie

    ledsToShow.push_back(11);
    ledsToShow.push_back(12);

    ledsToShow.push_back(31);
    ledsToShow.push_back(50);
    ledsToShow.push_back(51);
    ledsToShow.push_back(69);
    ledsToShow.push_back(70);

  } else if (minutes >= 15) { // et quart

    ledsToShow.push_back(9);
    ledsToShow.push_back(14);

    ledsToShow.push_back(33);
    ledsToShow.push_back(48);
    ledsToShow.push_back(53);
    ledsToShow.push_back(67);
    ledsToShow.push_back(72);
  }
}

void minutesPixels(int minutes) {
  switch (minutes) {
    case 1:
      ledsToShow.push_back(1);
      break;
    case 2: 
      ledsToShow.push_back(1);
      ledsToShow.push_back(98);
      break;
    case 3: 
      ledsToShow.push_back(1);
      ledsToShow.push_back(98);
      ledsToShow.push_back(105);
      break;
    case 4: 
      ledsToShow.push_back(1);
      ledsToShow.push_back(98);
      ledsToShow.push_back(105);
      ledsToShow.push_back(0);
      break;
    default:
      break;
  }

  // SERIAL_PORT_MONITOR.println();

}

void permanentsPixels() {
  ledsToShow.push_back(2); // I
  ledsToShow.push_back(21); // L
  ledsToShow.push_back(40); // E
  ledsToShow.push_back(41); // S
  ledsToShow.push_back(60); // T
  ledsToShow.push_back(55); // H
  ledsToShow.push_back(65); // E
  ledsToShow.push_back(74); // U
  ledsToShow.push_back(85); // R
  ledsToShow.push_back(92); // E
  ledsToShow.push_back(103); // S
}


// Do NOT use delay(), it breaks systemClock.loop()
void showLed() {
  ledsToShow.clear();
  // for (int i = 0; i < NUM_PIXELS; i++) {
  //   leds[i] = CRGB::Black; // Set Color HERE!!!
  // }
  FastLED.show();
  FastLED.clear();

  timeClient.update();

  permanentsPixels();
  hoursPixels(timeClient.getHours() % 12, ((timeClient.getMinutes() % 60) > 34));
  largeMinutesPixels(timeClient.getMinutes() % 60);
  minutesPixels(timeClient.getMinutes() % 5);
  
  for (int i = 0; i < ledsToShow.size(); i++) {
    leds[ledsToShow[i]] = CRGB::White;
  }

  FastLED.show();
}


void loop() {

  // PRINCIPAL
  if (modeSettings > 2) modeSettings = 0;

  if (digitalRead(buttonPrincipalPin) == LOW) {
    buttonPrincipalPressed = true;
  }

  if (digitalRead(buttonPrincipalPin) == HIGH && buttonPrincipalPressed) {
    modeSettings++;
    buttonPrincipalPressed = false;
  }

  if (modeSettings == CLOCK_SETTINGS) {

    if (clockSet > 2) clockSet = 0;

    // UP
    if (digitalRead(buttonUpPin) == LOW) {
      buttonUpPressed = true;
    }

    if (digitalRead(buttonUpPin) == HIGH && buttonUpPressed) {
      clockSet++;
      buttonUpPressed = false;
    }


    if (digitalRead(buttonDownPin) == LOW) {
      buttonDownPressed = true;
    }

    if (digitalRead(buttonDownPin) == HIGH && buttonDownPressed) {
      if (clockSet == HOURS_SETTINGS) {
        // hoursAdded++;
      } else if (clockSet == LARGE_MINUTES_SETTINGS) {
        // minutesAdded = minutesAdded + 5;
      } else if (clockSet == MINUTES_SETTINGS) {
        // minutesAdded++;
      }

      buttonDownPressed = false;
    }

  } else if (modeSettings == NORMAL_MODE) {

    if (digitalRead(buttonUpPin) == LOW) {
      buttonUpPressed = true;
    }

    if (digitalRead(buttonUpPin) == HIGH && buttonUpPressed) {
      if (brightness < 150) {
        brightness += 10;
      }
      buttonUpPressed = false;
    }

    if (digitalRead(buttonDownPin) == LOW) {
      buttonDownPressed = true;
    }

    if (digitalRead(buttonDownPin) == HIGH && buttonDownPressed) {
      if (brightness >= 20) {
        brightness -= 10;
      }
      buttonDownPressed = false;
    }
  }

  // if (modeSettings == CLOCK_SETTINGS) {
  //   // timeRefresh = 1000;
  //   // setClock();
  // } else if (modeSettings == COLOR_SETTINGS) {
  //   hardRandomColorMode = true;
  //   showLed();
  // } else {
  //   hardRandomColorMode = false;
  //   showLed();
  // }
  showLed();
  delay(timeRefresh);
}