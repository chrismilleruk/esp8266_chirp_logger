/*
 * Copyright (c) 2018, Chris Miller
 * All rights reserved.
 *
 */

#include "main.h"

const int led = 0;
const int interval_ms = 60000;

unsigned long startTime = 0;
unsigned long wakeupTime = 0;

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
