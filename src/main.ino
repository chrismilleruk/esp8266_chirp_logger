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
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "secrets.h"
// const char *wifi_ssid = "***";
// const char *wifi_password = "***";
// #define initialstate_bucket_key "***"
// #define initialstate_access_key "***"

ESP8266WebServer server ( 80 );

const int led = 0;
const int interval_ms = 60000;

#include <Wire.h>
#include "Adafruit_FRAM_I2C.h"
#include "Adafruit_MCP9808.h"
#include <Adafruit_BMP085_U.h>
#include <I2CSoilMoistureSensor.h>

/* Example code for the Adafruit I2C FRAM breakout */
Adafruit_FRAM_I2C fram     = Adafruit_FRAM_I2C();
uint16_t          framAddr = 0;

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
//Chirp_Sensor_Unified chirp = Chirp_Sensor_Unified(0x21);
I2CSoilMoistureSensor chirp(0x21);



// Navigate to http://esp8266.local/fram.txt


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
  unsigned long timestamp;
  chirp_sensor_t * chirp;
  bmp085_sensor_t * bmp085;
  mcp9808_sensor_t * mcp9808;
  float vccVoltage;
//  int hallSensor;
} sensor_values_t;


#include <ESP8266HTTPClient.h>

#define _InitialStateDomain "https://groker.initialstate.com"
#define _InitialStateUrl "/api/events"

int addr = 0x10;
const int addrStart = addr;
const int addrStep = 16;
const int addrLimit = addrStep * 256;

boolean fram_detected = false;

unsigned long startTime = 0;
unsigned long wakeupTime = 0;

void setup ( void ) {
  startTime = millis();

	pinMode ( led, OUTPUT );
	digitalWrite ( led, 0 );
	Serial.begin ( 74880 );
	WiFi.mode ( WIFI_STA );
	WiFi.begin ( wifi_ssid, wifi_password );
	Serial.println ( "" );

	// Wait for connection
	while ( WiFi.status() != WL_CONNECTED ) {
		delay ( 500 );
		Serial.print ( "." );
	}

	Serial.println ( "" );
	Serial.print ( "Connected to " );
	Serial.println ( wifi_ssid );
	Serial.print ( "IP address: " );
	Serial.println ( WiFi.localIP() );

	if ( MDNS.begin ( "esp8266" ) ) {
		Serial.println ( "MDNS responder started" );
	}

	// server.on ( "/", handleRoot );
	// server.on ( "/test.svg", drawGraph );
  // server.on ( "/fram.txt", sendFRAM );
	// server.on ( "/inline", []() {
	// 	server.send ( 200, "text/plain", "this works as well" );
	// } );
	// server.onNotFound ( handleNotFound );
	// server.begin();
	// Serial.println ( "HTTP server started" );

  setupStorage();
  setupSensors();

}

void loop ( void ) {
  if (wakeupTime > startTime) {
    startTime = wakeupTime;
  }
	server.handleClient();

  sensor_values_t values = readSensors();
  printSensorData(&values);
  storeSensorData(&values);
  sendSensorData(&values);

  unsigned long now = millis();
  wakeupTime = startTime + interval_ms;
  long waitTime = wakeupTime > now ? wakeupTime - now : 0;

  Serial.print("Going to sleep for ");
  Serial.print(waitTime);
  Serial.println("ms");

//  delay(waitTime);
  ESP.deepSleep(waitTime * 1000);
}
