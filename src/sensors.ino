
#include <StackThunk.h>

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 mcp9808 = Adafruit_MCP9808();
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
I2CSoilMoistureSensor chirp1(0x21);
I2CSoilMoistureSensor chirp2(0x22);
I2CSoilMoistureSensor chirp3(0x23);
I2CSoilMoistureSensor chirp4(0x24);

sensor_values_t sensor_values;
bmp085_sensor_t bmp085_values;
mcp9808_sensor_t mcp9808_values;
chirp_sensor_t chirp1_values;
chirp_sensor_t chirp2_values;
chirp_sensor_t chirp3_values;
chirp_sensor_t chirp4_values;

boolean bmp085_detected = false;
boolean mcp9808_detected = false;
boolean chirp1_detected = false;
boolean chirp2_detected = false;
boolean chirp3_detected = false;
boolean chirp4_detected = false;

ADC_MODE(ADC_VCC);

void setupSensors() {
  sensor_values.bmp085 = &bmp085_values;
  sensor_values.mcp9808 = &mcp9808_values;
  sensor_values.chirp1 = &chirp1_values;
  sensor_values.chirp2 = &chirp2_values;
  sensor_values.chirp3 = &chirp3_values;
  sensor_values.chirp4 = &chirp4_values;

  // Make sure the sensor is found, you can also pass in a different i2c
  // address with mcp9808.begin(0x19) for example
  Serial.print("🌱 Setup MCP9808.. ");
  if (mcp9808.begin()) {
    mcp9808_detected = true;
    Serial.println("OK");
  } else {
    Serial.println("Oops, no MCP9808 detected!");
  }

  /* Initialise the sensor */
  Serial.print("🌱 Setup BMP085.. ");
  if(bmp.begin()) {
    bmp085_detected = true;
    Serial.println("OK");
  } else {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.println("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
  }

  Serial.print("🌱 Setup chirp(s).. ");
  chirp1.begin();
  chirp2.begin();
  chirp3.begin();
  chirp4.begin();
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
  
  ch_fw = chirp3.getVersion();
  if (ch_fw == 0x23) {
    chirp3_detected = true;
    Serial.print("(3) ");
  }
  
  ch_fw = chirp4.getVersion();
  if (ch_fw == 0x23) {
    chirp4_detected = true;
    Serial.print("(4) ");
  }
  
  if (chirp1_detected || chirp2_detected || chirp3_detected || chirp4_detected) {
    Serial.println("OK");
  } else {
   /* There was a problem detecting the Chirp */
   Serial.println("Ooops, Chirps not detected.");
 }

  // sensor_t sensor;

  // if (bmp085_detected) {
  //   bmp.getSensor(&sensor);
  //   Serial.println("------------------------------------");
  //   Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  //   Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  //   Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  //   Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
  //   Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
  //   Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");
  //   Serial.println("------------------------------------");
  //   Serial.println("");
  // }
}



void printChirpSensorData(chirp_sensor_t * chirp) {
  Serial.printf("Temp:\t🌡 %.2f*C\tMoist:\t💧%.1f\tLight:\t☀️ %.1f\n",
    chirp->temperature,
    chirp->moisture,
    chirp->light
  );
}

void printSensorData(sensor_values_t * sensors) {

  Serial.println();
  Serial.print("🌱 ESP8266\t");
  Serial.print("Volts:\t⚡️");
  Serial.print(sensors->vccVoltage);
  Serial.print("V");
//  Serial.print("\tHall:\t");
//  Serial.print(sensors->hallSensor);
  Serial.print("\t\tReset reason:\t⏏");
  Serial.println(reset_info->reason);

  if (bmp085_detected) {
    /* Display atmospheric pressue in hPa */
    Serial.print("🌱 BMP085\t");
    Serial.print("Temp:\t🌡 ");
    Serial.print(sensors->bmp085->temperature);
    Serial.print("*C\t");
    Serial.print("Pressure:\t🪂 ");
    Serial.print(sensors->bmp085->pressure);
    Serial.println(" hPa");
  }

  if (mcp9808_detected) {
    Serial.print("🌱 MCP9808\tTemp:\t🌡 ");
    Serial.print(sensors->mcp9808->temperature);
    Serial.println("*C");
  }

  if (chirp1_detected) {
    Serial.print("🌱 chirp1\t");
    printChirpSensorData(sensors->chirp1);
  }

  if (chirp2_detected) {
    Serial.print("🌱 chirp2\t");
    printChirpSensorData(sensors->chirp2);
  }

  if (chirp3_detected) {
    Serial.print("🌱 chirp3\t");
    printChirpSensorData(sensors->chirp3);
  }

  if (chirp4_detected) {
    Serial.print("🌱 chirp4\t");
    printChirpSensorData(sensors->chirp4);
  }

}

void sendSensorData(sensor_values_t * sensors) {
  Serial.println();

  static const char* host = "groker.init.st";
  int port = 443;
  String url = "/api/events";
  url += "?accessKey=" + String(initialstate_access_key);
  url += "&bucketKey=" + String(initialstate_bucket_key);

  // *.initialstate.com
  // *.init.st
  // Expires: Sunday, 7 March 2021 at 12:00:00 Greenwich Mean Time
  // SHA 256  EF 25 4B 79 96 E1 EE BB A4 82 F6 CB B2 16 02 DC F1 2B C0 EB 9E 10 30 F5 A1 75 7E 23 63 BA 66 AB
  // SHA-1    26 79 30 97 D3 DF 5A 60 55 C2 FC 29 B4 12 8B 56 28 B4 E6 05
  static const char* fingerprint PROGMEM = "26 79 30 97 D3 DF 5A 60 55 C2 FC 29 B4 12 8B 56 28 B4 E6 05";

  // Invalid Fingerprint for testing.
  // static const char* fingerprint PROGMEM = "BA D0 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF"; 
  
  if ( WiFi.status() == WL_CONNECTED ) {

    // Track memory usage
    ESP.resetFreeContStack();
    uint32_t freeStackStart = ESP.getFreeContStack();

    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    boolean isSecure = (port == 443);

    Serial.print("🔒 connecting to ");
    Serial.println(host);

    // We will test the fingerprint of the SSL cert to ensure it matches.
    if (isSecure) {
      client.setFingerprint(fingerprint);

      if (client.connect(host, port)) {
        Serial.println("🔐 secure connection made");
      } else {
        Serial.println("🔐 secure connection failed");

        // This could be because the SSL cert has been renewed, and the fingerprint has changed.
        // Try insecure connection instead.
        isSecure = false;
      }
    }

    if (!isSecure) {
      // Using setInsecure will not validate the SSL cert in any way.
      client.setInsecure();

      if (client.connect(host, port)) {
        Serial.printf("🔒⚠️  insecure connection on port %d\n", port);
      } else {
        Serial.println("🔒❌ connection failed");
        return;
      }
    }


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
    url += "&isSecure=" + String(isSecure);

    Serial.print("🌐 URL: ");
    Serial.println(url);

    Serial.print("🌐 Request ");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");

    int repeatCounter = 10;
    while (!client.available() && repeatCounter--) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("sent 🚀");
    
    String line;
    line = client.readStringUntil('\n');

    Serial.println("🌐 Response:");
    Serial.println(line);

    client.stop();
    Serial.println("🔐 Connection Closed");
      
    // Track memory usage
    uint32_t freeStackEnd = ESP.getFreeContStack();
    Serial.printf("\n💻 CONT stack used: %d\n", freeStackStart - freeStackEnd);
    Serial.printf("💻 BSSL stack used: %d\n-------\n\n", stack_thunk_get_max_usage());
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
    mcp9808_values.temperature = mcp9808.readTempC();
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

  if (chirp3_detected) {
    /* Read the chirp3 values */
    chirp3_values.moisture = chirp3.getCapacitance();
    chirp3_values.temperature = (float)chirp3.getTemperature() / 10;
    chirp3_values.light = chirp3.getLight(true);
  }

  if (chirp4_detected) {
    /* Read the chirp4 values */
    chirp4_values.moisture = chirp4.getCapacitance();
    chirp4_values.temperature = (float)chirp4.getTemperature() / 10;
    chirp4_values.light = chirp4.getLight(true);
  }

  sensor_values.vccVoltage = ((float)ESP.getVcc())/1024;
//  sensor_values.hallSensor = hallRead();

  return sensor_values;
}
