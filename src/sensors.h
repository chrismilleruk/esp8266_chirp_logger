#ifndef SENSORS_H
#define SENSORS_H

#include "main.h"

#include <Wire.h>
#include "Adafruit_FRAM_I2C.h"
#include "Adafruit_MCP9808.h"
#include <Adafruit_BMP085_U.h>
#include <I2CSoilMoistureSensor.h>
#include "Adafruit_MCP23017.h"

void setRAG(uint8_t red, uint8_t amber, uint8_t green);

void setupRAG();

void setupSensors();

// void printChirpSensorData(chirp_sensor_t * chirp);

sensor_values_t readSensors(boolean printSensorData);

void sendSensorData(sensor_values_t * sensors);

#endif