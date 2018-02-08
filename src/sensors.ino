
// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
I2CSoilMoistureSensor chirp1(0x21);
I2CSoilMoistureSensor chirp2(0x22);

sensor_values_t sensor_values;
bmp085_sensor_t bmp085_values;
mcp9808_sensor_t mcp9808_values;
chirp_sensor_t chirp1_values;
chirp_sensor_t chirp2_values;

boolean bmp085_detected = false;
boolean mcp9808_detected = false;
boolean chirp1_detected = false;
boolean chirp2_detected = false;

ADC_MODE(ADC_VCC);

void setupSensors() {
  sensor_values.bmp085 = &bmp085_values;
  sensor_values.mcp9808 = &mcp9808_values;
  sensor_values.chirp1 = &chirp1_values;
  sensor_values.chirp2 = &chirp2_values;

  // Make sure the sensor is found, you can also pass in a different i2c
  // address with tempsensor.begin(0x19) for example
  Serial.print("Setup MCP9808.. ");
  if (tempsensor.begin()) {
    mcp9808_detected = true;
    Serial.println("OK");
  } else {
    Serial.println("Oops, no MCP9808 detected!");
//    while (1) { yield; }
  }

  /* Initialise the sensor */
  Serial.print("Setup BMP085.. ");
  if(bmp.begin()) {
    bmp085_detected = true;
    Serial.println("OK");
  } else {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.println("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
//    while(1) { yield; }
  }

  Serial.print("Setup chirp(s).. ");
  chirp1.begin();
  chirp2.begin();
  delay(1000); // give some time to boot up
  uint8_t ch_fw = chirp1.getVersion();
  if (ch_fw == 0x23) {
    chirp1_detected = true;
    Serial.print("(1) ");
  }
  ch_fw = chirp2.getVersion();
  if (ch_fw == 0x23) {
    chirp2_detected = true;
    Serial.print("(2) ");
  }
  if (chirp1_detected && chirp2_detected) {
    Serial.println("OK");
  } else {
   /* There was a problem detecting the Chirp */
   Serial.println("Ooops, Chirps not detected.");
 }

  sensor_t sensor;

  if (bmp085_detected) {
    bmp.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");
    Serial.println("------------------------------------");
    Serial.println("");
  }
}



void printSensorData(sensor_values_t * sensors) {

  Serial.print("Voltage:\t");
  Serial.print(sensors->vccVoltage);
  Serial.print("V");
//  Serial.print("\tHall:\t");
//  Serial.print(sensors->hallSensor);
  Serial.print("\tReset reason:\t");
  Serial.println(reset_info->reason);

  if (bmp085_detected) {
    /* Display atmospheric pressue in hPa */
    Serial.print("BMP Pressure:\t");
    Serial.print(sensors->bmp085->pressure);
    Serial.print(" hPa");
    Serial.print("\tTemp:\t");
    Serial.print(sensors->bmp085->temperature);
    Serial.print("*C");
  }

  if (mcp9808_detected) {
    Serial.print("\tMCP Temp:\t");
    Serial.print(sensors->mcp9808->temperature);
    Serial.println("*C");
  }

  if (chirp1_detected) {
    Serial.print("chirp1 Temp:\t");
    Serial.print(sensors->chirp1->temperature);
    Serial.print("*C");
    Serial.print("\tMoist:\t");
    Serial.print(sensors->chirp1->moisture);
    Serial.print("\tLight:\t");
    Serial.println(sensors->chirp1->light);
  }

  if (chirp2_detected) {
    Serial.print("chirp2 Temp:\t");
    Serial.print(sensors->chirp2->temperature);
    Serial.print("*C");
    Serial.print("\tMoist:\t");
    Serial.print(sensors->chirp2->moisture);
    Serial.print("\tLight:\t");
    Serial.println(sensors->chirp2->light);
  }

}

void sendSensorData(sensor_values_t * sensors) {
  if (!bmp085_detected && !mcp9808_detected && !chirp1_detected && !chirp2_detected) {
    Serial.println("No sensor data to send");
    return;
  }

  if ( WiFi.status() == WL_CONNECTED ) {

    const char* host = "groker.initialstate.com";
    // const char* fingerprint = "D2 DF 3E 80 5F 89 0C 7D 9D D2 10 B4 A7 21 82 C8 C8 43 FE 0D";
    const char* fingerprint = "9C 81 BD 65 44 89 2A 68 FB 33 B5 F1 7F 3F 1D 16 AC 11 B6 23";
    int httpsPort = 443;

    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.print("connecting to ");
    Serial.println(host);

    if (!client.connect(host, httpsPort)) {
      Serial.println("connection failed");
      return;
    }

    if (!client.verify(fingerprint, host)) {
      Serial.println("Certificate doesn't match");
    }

    String url = _InitialStateUrl;
    url += "?accessKey=" + String(initialstate_access_key);
    url += "&bucketKey=" + String(initialstate_bucket_key);
    if (mcp9808_detected) {
      url += "&temp_c=" + String(sensors->mcp9808->temperature);
    }
    if (bmp085_detected) {
      url += "&temp_b=" + String(sensors->bmp085->temperature);
      url += "&pressure_b=" + String(sensors->bmp085->pressure);
    }
    if (chirp1_detected) {
      url += "&temp_chirp1=" + String(sensors->chirp1->temperature);
      url += "&light_chirp1=" + String(sensors->chirp1->light);
      url += "&moisture_chirp1=" + String(sensors->chirp1->moisture);
    }
    if (chirp2_detected) {
      url += "&temp_chirp2=" + String(sensors->chirp2->temperature);
      url += "&light_chirp2=" + String(sensors->chirp2->light);
      url += "&moisture_chirp2=" + String(sensors->chirp2->moisture);
    }
    url += "&vcc_volt=" + String(sensors->vccVoltage);
//    url += "&hall_read=" + String(sensors->hallSensor);

    Serial.print("requesting URL: ");
    Serial.println(url);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");
    Serial.println("request sent");
    int repeatCounter = 10;
    while (!client.available() && repeatCounter--) {
      delay(500);
    }
    String line;
    line = client.readStringUntil('\n');
    Serial.println("reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("closing connection");
  }
}

sensor_values_t readSensors() {
  sensor_values.timestamp = millis();

  sensors_event_t event;
  float temperature;
  float light;

  if (bmp085_detected) {
    /* Read the pressure event and current temperature from the BMP085 */
    bmp.getEvent(&event);
    bmp.getTemperature(&temperature);
    bmp085_values.temperature = temperature;
    bmp085_values.pressure = event.pressure;
  }

  if (mcp9808_detected) {
    /* Read the temperature from the MCP9808 */
    mcp9808_values.temperature = tempsensor.readTempC();
  }

  if (chirp1_detected) {
    /* Read the chirp1 values */
    chirp1_values.moisture = chirp1.getCapacitance();
    chirp1_values.temperature = (float)chirp1.getTemperature() / 10;
    chirp1_values.light = chirp1.getLight(true);
  }

  if (chirp2_detected) {
    /* Read the chirp2 values */
    chirp2_values.moisture = chirp2.getCapacitance();
    chirp2_values.temperature = (float)chirp2.getTemperature() / 10;
    chirp2_values.light = chirp2.getLight(true);
  }

  sensor_values.vccVoltage = ((float)ESP.getVcc())/1024;
//  sensor_values.hallSensor = hallRead();

  return sensor_values;
}
