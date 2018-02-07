
sensor_values_t sensor_values;
bmp085_sensor_t bmp085_values;
mcp9808_sensor_t mcp9808_values;
chirp_sensor_t chirp_values;

boolean bmp085_detected = false;
boolean mcp9808_detected = false;
boolean chirp_detected = false;

ADC_MODE(ADC_VCC);

void setupSensors() {
  sensor_values.bmp085 = &bmp085_values;
  sensor_values.mcp9808 = &mcp9808_values;
  sensor_values.chirp = &chirp_values;

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

  Serial.print("Setup Chirp.. ");
  chirp.begin();
  delay(1000); // give some time to boot up
  uint8_t ch_fw = chirp.getVersion();
  if (ch_fw == 0x23) {
    chirp_detected = true;
    Serial.println("OK");
  } else {
    /* There was a problem detecting the Chirp */
    Serial.println("Ooops, no Chirp detected ... Check your wiring or I2C ADDR!");
//    while (1) { yield; }
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

  if (chirp_detected) {
    Serial.print("Chirp Temp:\t");
    Serial.print(sensors->chirp->temperature);
    Serial.print("*C");
    Serial.print("\tMoist:\t");
    Serial.print(sensors->chirp->moisture);
    Serial.print("\tLight:\t");
    Serial.println(sensors->chirp->light);
  }

}

void storeSensorData(sensor_values_t * sensors) {
  if (fram_detected) {
    uint16_t val = sensors->timestamp * 0xFF / 1000;
    framWrite16(addr, val);
    val = (uint16_t) (sensors->mcp9808->temperature * 0x100); // AB.cd
    framWrite16(addr+2, val);
    val = (uint16_t) (sensors->bmp085->temperature * 0x100); // AB.cd
    framWrite16(addr+4, val);
    val = (uint16_t) (sensors->bmp085->pressure * 0x10); // ABC.d
    framWrite16(addr+6, val);

    val = (uint16_t) (sensors->chirp->temperature * 0x100); // AB.cd
    framWrite16(addr+8, val);
    val = (uint16_t) (sensors->chirp->moisture * 0x100); // AB.cd
    framWrite16(addr+10, val);
    val = (uint16_t) (sensors->chirp->light * 0x100); // AB.cd
    framWrite16(addr+12, val);

    Serial.print(addr);
    for (uint8_t a = 0; a < addrStep; a+=1) {
      if (a % 2 == 0) Serial.print("\t");
      byte b = fram.read8(addr+a);
      if (b < 0x10) Serial.print(0);
      Serial.print(fram.read8(addr+a), HEX);
    }

    Serial.println();

    // advance to the next address.  there are 512 bytes in
    // the EEPROM, so go back to 0 when we hit 512.
    // save all changes to the flash.
    addr = addr + addrStep;
    if (addr + addrStep >= addrLimit)
    {
      addr = addrStart;
    }
  }
}

void sendSensorData(sensor_values_t * sensors) {
  if (!bmp085_detected && !mcp9808_detected && !chirp_detected) {
    Serial.println("No sensor data to send");
    return;
  }

  if ( WiFi.status() == WL_CONNECTED ) {

    String host = "groker.initialstate.com";
    String fingerprint = "D2 DF 3E 80 5F 89 0C 7D 9D D2 10 B4 A7 21 82 C8 C8 43 FE 0D";
    int httpsPort = 443;

    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
      Serial.println("connection failed");
      return;
    }

    //if (client.verify(fingerprint, host)) {
    //Serial.println("Certificate matches");
    //} else {
    //Serial.println("Certificate doesn't match");
    //}

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
    if (chirp_detected) {
      url += "&temp_chirp=" + String(sensors->chirp->temperature);
      url += "&light_chirp=" + String(sensors->chirp->light);
      url += "&moisture_chirp=" + String(sensors->chirp->moisture);
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

  if (chirp_detected) {
    /* Read the chirp values */
    chirp_values.moisture = chirp.getCapacitance();
    chirp_values.temperature = (float)chirp.getTemperature() / 10;
    chirp_values.light = chirp.getLight(true);
  }

  sensor_values.vccVoltage = ((float)ESP.getVcc())/1024;
//  sensor_values.hallSensor = hallRead();

  return sensor_values;
}
