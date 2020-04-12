#ifndef TYPEDEFS_H
#define TYPEDEFS_H

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

#endif