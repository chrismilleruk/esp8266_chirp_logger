
void setupStorage() {
  Serial.print("I2C FRAM... ");
  if (fram.begin()) {  // alternate i2c addr, e.g. begin(0x51);
    Serial.println("Found");
    fram_detected = true;
  } else {
    Serial.println("Not found ... check your connections\r\n");
//    while (1) { yield; }
  }

  if (fram_detected) {
    // Read the first byte
    uint8_t test = fram.read8(0x0);
    Serial.print("Restarted "); Serial.print(test); Serial.println(" times");
    // Test write ++
    fram.write8(0x0, test+1);
  }
}

void framWrite16(int addr, uint16_t val) {
  fram.write8(addr+0, (val >> 8)  & 0xFF);
  fram.write8(addr+1, val & 0xFF);
}

