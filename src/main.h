#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

#include "typedefs.h"
#include "secrets.h"
// #define wifi_ssid "***"
// #define wifi_password "***"
// #define initialstate_bucket_key "***"
// #define initialstate_access_key "***"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

// Navigate to http://esp8266.local/fram.txt

#define _InitialStateDomain "https://groker.initialstate.com"
#define _InitialStateUrl "/api/events"

#include "network.h"
#include "sensors.h"
#include "storage.h"

#endif