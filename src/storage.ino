
/* Example code for the Adafruit I2C FRAM breakout */
Adafruit_FRAM_I2C fram     = Adafruit_FRAM_I2C();
uint16_t          framAddr = 0;

boolean fram_detected = false;


void setupStorage() {
  Serial.print("📀 I2C FRAM... ");
  if (fram.begin()) {  // alternate i2c addr, e.g. begin(0x51);
    Serial.println("Found");
    fram_detected = true;
  } else {
    Serial.println("Not found ... check your connections\r\n");
  }

  if (fram_detected) {
    // Read the first byte
    uint8_t test = fram.read8(0x0);
    Serial.print("📀 Restarted "); Serial.print(test); Serial.println(" times");
    // Test write ++
    fram.write8(0x0, test+1);
  }
}

void framWrite16(int addr, uint16_t val) {
  fram.write8(addr+0, (val >> 8)  & 0xFF);
  fram.write8(addr+1, val & 0xFF);
}


void storeSensorData(sensor_values_t * sensors) {
  if (fram_detected) {
    uint16_t val = sensors->timestamp * 0xFF / 1000;
    framWrite16(addr, val);
    // val = (uint16_t) (sensors->mcp9808->temperature * 0x100); // AB.cd
    // framWrite16(addr+2, val);
    // val = (uint16_t) (sensors->bmp085->temperature * 0x100); // AB.cd
    // framWrite16(addr+4, val);
    // val = (uint16_t) (sensors->bmp085->pressure * 0x10); // ABC.d
    // framWrite16(addr+6, val);
    val = (uint16_t) (sensors->chirp1->temperature * 0x100); // AB.cd
    framWrite16(addr+2, val);
    val = (uint16_t) (sensors->chirp1->moisture * 0x100); // AB.cd
    framWrite16(addr+4, val);
    val = (uint16_t) (sensors->chirp1->light * 0x100); // AB.cd
    framWrite16(addr+6, val);
    val = (uint16_t) (sensors->chirp2->temperature * 0x100); // AB.cd
    framWrite16(addr+8, val);
    val = (uint16_t) (sensors->chirp2->moisture * 0x100); // AB.cd
    framWrite16(addr+10, val);
    val = (uint16_t) (sensors->chirp2->light * 0x100); // AB.cd
    framWrite16(addr+12, val);
    framWrite16(addr+14, 0x0);

    Serial.print("📀 ");
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
