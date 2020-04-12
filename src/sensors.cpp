#include "main.h"

#ifdef ESP8266
extern "C" {
  #include "user_interface.h"
}
#endif

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 mcp9808 = Adafruit_MCP9808();
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
I2CSoilMoistureSensor chirp1(0x21);
I2CSoilMoistureSensor chirp2(0x22);
I2CSoilMoistureSensor chirp3(0x23);
I2CSoilMoistureSensor chirp4(0x24);
Adafruit_MCP23017 mcp23017;

sensor_values_t sensor_values;
bmp085_sensor_t bmp085_values;
mcp9808_sensor_t mcp9808_values;
mcp23017_sensor_t mcp23017_values;
chirp_sensor_t chirp1_values;
chirp_sensor_t chirp2_values;
chirp_sensor_t chirp3_values;
chirp_sensor_t chirp4_values;

ADC_MODE(ADC_VCC);

// The following required by sendSensorData:
#include <StackThunk.h>
boolean mcp23017_detected = false;
boolean bmp085_detected = false;
boolean mcp9808_detected = false;
boolean chirp1_detected = false;
boolean chirp2_detected = false;
boolean chirp3_detected = false;
boolean chirp4_detected = false;
struct rst_info *reset_info = system_get_rst_info();
unsigned int deviceCount = 0;


void setRAG(uint8_t red, uint8_t amber, uint8_t green) {
  if (mcp23017_detected) {
    Serial.print("🚥 ");
    mcp23017.digitalWrite(0, red);
    Serial.print(red ? "🔴 " : "⚪️ ");
    mcp23017.digitalWrite(1, amber);
    Serial.print(amber ? "🟠 " : "⚪️ ");
    mcp23017.digitalWrite(2, green);
    Serial.print(green ? "🟢 " : "⚪️ ");
    Serial.println();
  }
}

void setupRAG() {

  // The MCP23017 is a 16-bit IO port expander.
  // pinMode(): Pins numbered from 0 to 7 are on Port A, and pins numbered from 8 to 15 are on Port B.
  // readGPIOAB()/writeGPIOAB(): The LSB corresponds to Port A, pin 0, and the MSB corresponds to Port B, pin 7.
  // In order to check that it is present we need to write some values and read them.
  // The water level sensors are on Port B so we just check 
  Serial.print("🌱 Setup MCP23017.. ");
  mcp23017.begin();
  Serial.print(". ");
  for (int p = 0; p < 8; p +=1) {
    mcp23017.pinMode(p, OUTPUT);
    mcp23017.digitalWrite(p, HIGH);
    Serial.print(".");
  }
  Serial.print(". ");
  for (int p = 8; p < 16; p +=1) {
    mcp23017.pinMode(p, INPUT);
    // Use external pullups for pins 8-10
    if (p > 10) {
      mcp23017.pullUp(p, HIGH);
    }
    Serial.print(".");
  }
  Serial.print(".");
  mcp23017.writeGPIOAB(0x0000);
  uint16_t ioValues = mcp23017.readGPIOAB();
  Serial.printf("💧 0x%x ", ioValues);
  if ((ioValues & 0xF) == 0x0) {
    mcp23017_detected = true;
    deviceCount += 1;
    Serial.println("OK");
  } else {
    Serial.println("Oops, no MCP23017 detected!");
  }
  mcp23017.writeGPIOAB(0x0000);

}

void setupSensors() {
  sensor_values.bmp085 = &bmp085_values;
  sensor_values.mcp9808 = &mcp9808_values;
  sensor_values.mcp23017 = &mcp23017_values;
  sensor_values.chirp1 = &chirp1_values;
  sensor_values.chirp2 = &chirp2_values;
  sensor_values.chirp3 = &chirp3_values;
  sensor_values.chirp4 = &chirp4_values;

  // Make sure the sensor is found, you can also pass in a different i2c
  // address with mcp9808.begin(0x19) for example
  Serial.print("🌱 Setup MCP9808.. ");
  if (mcp9808.begin()) {
    mcp9808_detected = true;
    deviceCount += 1;
    Serial.println("OK");
  } else {
    Serial.println("Oops, no MCP9808 detected!");
  }

  /* Initialise the sensor */
  Serial.print("🌱 Setup BMP085.. ");
  if(bmp.begin()) {
    bmp085_detected = true;
    deviceCount += 1;
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
    deviceCount += 1;
    Serial.print("(1) ");
  }

  ch_fw = chirp2.getVersion();
  if (ch_fw == 0x23) {
    chirp2_detected = true;
    deviceCount += 1;
    Serial.print("(2) ");
  }
  
  ch_fw = chirp3.getVersion();
  if (ch_fw == 0x23) {
    chirp3_detected = true;
    deviceCount += 1;
    Serial.print("(3) ");
  }
  
  ch_fw = chirp4.getVersion();
  if (ch_fw == 0x23) {
    chirp4_detected = true;
    deviceCount += 1;
    Serial.print("(4) ");
  }
  
  if (chirp1_detected || chirp2_detected || chirp3_detected || chirp4_detected) {
    Serial.println("OK");
  } else {
   /* There was a problem detecting the Chirp */
   Serial.println("Ooops, Chirps not detected.");
 }

  Serial.printf("🌱 Found %d devices\n", deviceCount);
}

void printChirpSensorData(chirp_sensor_t * chirp) {
  Serial.printf("Temp:\t🌡 %.2f*C\tMoist:\t💧%.1f\tLight:\t☀️ %.1f\n",
    chirp->temperature,
    chirp->moisture,
    chirp->light
  );
}

sensor_values_t readSensors(boolean printSensorData) {
  sensor_values.timestamp = millis();

  sensors_event_t event;
  float temperature;

  sensor_values.vccVoltage = ((float)ESP.getVcc())/1024;
  // sensor_values.hallSensor = hallRead();

  if (printSensorData) {
    Serial.println();
    Serial.print("🌱 ESP8266\t");
    Serial.print("Volts:\t⚡️");
    Serial.print(sensor_values.vccVoltage);
    Serial.print("V");
    // Serial.print("\tHall:\t");
    // Serial.print(sensor_values.hallSensor);
    Serial.print("\t\tReset reason:\t⏏");
    Serial.println(reset_info->reason);
  }

  if (bmp085_detected) {
    /* Read the pressure event and current temperature from the BMP085 */
    bmp.getEvent(&event);
    bmp.getTemperature(&temperature);
    bmp085_values.temperature = temperature;
    bmp085_values.pressure = event.pressure;

    if (printSensorData) {
      /* Display atmospheric pressue in hPa */
      Serial.print("🌱 BMP085\t");
      Serial.print("Temp:\t🌡 ");
      Serial.print(sensor_values.bmp085->temperature);
      Serial.print("*C\t");
      Serial.print("Pressure:\t🪂 ");
      Serial.print(sensor_values.bmp085->pressure);
      Serial.println(" hPa");
    }
  }

  if (mcp23017_detected) {
    // We're using this to determine the water level.
    // The 8 bits of Port B are set to INPUT & PULL_UP.
    // Wires are arranged in the tank as follows:
    // 5 4 3 2 1 GND
    // | | | | | |
    //   | | | | |
    //     | | | |
    //       | | |
    //         | |
    //           |

    // readGPIOAB(): The LSB corresponds to Port A, pin 0, and the MSB corresponds to Port B, pin 7.
    uint16_t ioValues = mcp23017.readGPIOAB();

    // Shift the LSB & Invert MSB to get the values on Port B.
    uint8_t ioValues2 = ~(ioValues >> 8);
    // 5 = 0b11111
    // 4 = 0b01111
    // 3 = 0b00111
    // 2 = 0b00011
    // 1 = 0b00001
    // 0 = 0b00000

    // Shift the bits to find the MSB
    // This means that failures such as 0b01101 will still report level 4
    uint8_t level = 0;
    while (ioValues2 > 0) {
      level += 1;
      ioValues2 = ioValues2 >> 1;
    }

    mcp23017_values.waterLevel = level;

    if (printSensorData) {
      Serial.print("🌱 MCP23017\t");
      Serial.print("Value:\t ");
      Serial.printf("\t0x%x ", (uint8_t)~(ioValues >> 8));
      Serial.printf("\tLevel:\t💧 %d", sensor_values.mcp23017->waterLevel);
    
      Serial.println("");
    }
  }

  if (mcp9808_detected) {
    /* Read the temperature from the MCP9808 */
    mcp9808_values.temperature = mcp9808.readTempC();

    if (printSensorData) {
      Serial.print("🌱 MCP9808\tTemp:\t🌡 ");
      Serial.print(sensor_values.mcp9808->temperature);
      Serial.println("*C");
    }
  }

  if (chirp1_detected) {
    /* Read the chirp1 values */
    chirp1_values.moisture = chirp1.getCapacitance();
    chirp1_values.temperature = (float)chirp1.getTemperature() / 10;
    chirp1_values.light = chirp1.getLight(true);

    if (printSensorData) {
      Serial.print("🌱 chirp1\t");
      printChirpSensorData(sensor_values.chirp1);
    }
  }

  if (chirp2_detected) {
    /* Read the chirp2 values */
    chirp2_values.moisture = chirp2.getCapacitance();
    chirp2_values.temperature = (float)chirp2.getTemperature() / 10;
    chirp2_values.light = chirp2.getLight(true);

    if (printSensorData) {
      Serial.print("🌱 chirp2\t");
      printChirpSensorData(sensor_values.chirp2);
    }
  }

  if (chirp3_detected) {
    /* Read the chirp3 values */
    chirp3_values.moisture = chirp3.getCapacitance();
    chirp3_values.temperature = (float)chirp3.getTemperature() / 10;
    chirp3_values.light = chirp3.getLight(true);

    if (printSensorData) {
      Serial.print("🌱 chirp3\t");
      printChirpSensorData(sensor_values.chirp3);
    }
  }

  if (chirp4_detected) {
    /* Read the chirp4 values */
    chirp4_values.moisture = chirp4.getCapacitance();
    chirp4_values.temperature = (float)chirp4.getTemperature() / 10;
    chirp4_values.light = chirp4.getLight(true);

    if (printSensorData) {
      Serial.print("🌱 chirp4\t");
      printChirpSensorData(sensor_values.chirp4);
    }
  }

  return sensor_values;
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
        setRAG(HIGH, HIGH, LOW);
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
        setRAG(HIGH, LOW, LOW);
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
    if (chirp3_detected) {
      url += "&temp_chirp3=" + String(sensors->chirp3->temperature);
      url += "&light_chirp3=" + String(sensors->chirp3->light);
      url += "&moisture_chirp3=" + String(sensors->chirp3->moisture);
    }
    if (chirp4_detected) {
      url += "&temp_chirp4=" + String(sensors->chirp4->temperature);
      url += "&light_chirp4=" + String(sensors->chirp4->light);
      url += "&moisture_chirp4=" + String(sensors->chirp4->moisture);
    }
    if (mcp23017_detected) {
      url += "&water_level=" + String(sensors->mcp23017->waterLevel);
    }
    //    url += "&hall_read=" + String(sensors->hallSensor);
    url += "&vcc_volt=" + String(sensors->vccVoltage);
    url += "&resetReason=" + String(reset_info->reason);
    url += "&deviceCount=" + String(deviceCount);
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
