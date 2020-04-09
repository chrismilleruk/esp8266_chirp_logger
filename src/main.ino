/*
 * Copyright (c) 2018, Chris Miller
 * All rights reserved.
 *
 */
#ifdef ESP8266
extern "C" {
  #include "user_interface.h"
}
#endif

struct rst_info *reset_info = system_get_rst_info();

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "secrets.h"
// const char *wifi_ssid = "***";
// const char *wifi_password = "***";
// #define initialstate_bucket_key "***"
// #define initialstate_access_key "***"

const int led = 0;
const int interval_ms = 60000;

#include <Wire.h>
#include "Adafruit_FRAM_I2C.h"
#include "Adafruit_MCP9808.h"
#include <Adafruit_BMP085_U.h>
#include <I2CSoilMoistureSensor.h>
#include "Adafruit_MCP23017.h"

// Navigate to http://esp8266.local/fram.txt


#include <ESP8266HTTPClient.h>

#define _InitialStateDomain "https://groker.initialstate.com"
#define _InitialStateUrl "/api/events"

int addr = 0x10;
const int addrStart = addr;
const int addrStep = 16;
const int addrLimit = addrStep * 256;

typedef struct {
  float temperature;
  float moisture;
  float light;
} chirp_sensor_t;

typedef struct {
  float temperature;
  float pressure;
} bmp085_sensor_t;

typedef struct {
  float temperature;
} mcp9808_sensor_t;

typedef struct {
  uint8_t waterLevel;
} mcp23017_sensor_t;

typedef struct {
  unsigned long timestamp;
  chirp_sensor_t * chirp1;
  chirp_sensor_t * chirp2;
  chirp_sensor_t * chirp3;
  chirp_sensor_t * chirp4;
  bmp085_sensor_t * bmp085;
  mcp9808_sensor_t * mcp9808;
  mcp23017_sensor_t * mcp23017;
  float vccVoltage;
//  int hallSensor;
} sensor_values_t;

unsigned long startTime = 0;
unsigned long wakeupTime = 0;
unsigned int deviceCount = 0;

void setup ( void ) {
  startTime = millis();

	pinMode ( led, OUTPUT );
	digitalWrite ( led, 0 );
	Serial.begin ( 74880 );
  setupRAG();

  setRAG(HIGH, LOW, LOW);
  setupNetwork();
  setupSensors();

  setRAG(HIGH, HIGH, LOW);
  setupStorage();

  setRAG(HIGH, HIGH, HIGH);
  waitForNetwork();
}

void loop ( void ) {
  // This line handles loops which don't involve deep sleep.
  if (wakeupTime > startTime) {
    startTime = wakeupTime;
  }

  setRAG(LOW, LOW, HIGH);
  sensor_values_t values = readSensors(true);
  
  setRAG(LOW, HIGH, LOW);
  storeSensorData(&values);
  sendSensorData(&values);

  unsigned long now = millis();
  wakeupTime = startTime + interval_ms;
  long waitTime = wakeupTime > now ? wakeupTime - now : 0;

  Serial.print("💤 Going to sleep for ");
  Serial.print(waitTime);
  Serial.println("ms");

  setRAG(LOW, LOW, LOW);
  ESP.deepSleep(waitTime * 1000);
}
