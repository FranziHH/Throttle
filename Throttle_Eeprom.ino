
void getAddresses(int offset) {
  int lsb = 0;
  for (int i = 0; i < maxLocos; i++) {
    LocoAddress[i] = EEPROM.read((i * 2) + offset) * 256;
    LocoAddress[i] = LocoAddress[i] + EEPROM.read((i * 2 + 1) + offset);
    if (LocoAddress[i] >= 10000) LocoAddress[i] = 3;
    #if debug == 1
      Serial.println(" ");
      Serial.print("loco = ");
      Serial.print(LocoAddress[i]);
      Serial.print("  address# = ");
      Serial.print(i + 1);
    #endif
  }
}

void getPushFunctions(int offset) {
  // read PushFunctions
  int next_addr = (2 * maxLocos) + offset;
  for (int i = 0; i < maxLocos; i++) {
    LocoPushFunction[i] = ((unsigned long)EEPROM.read(next_addr) << 24) +
                          ((unsigned long)EEPROM.read(next_addr + 1) << 16) +
                          ((unsigned long)EEPROM.read(next_addr + 2) << 8) +
                          (unsigned long)EEPROM.read(next_addr + 3);
    next_addr = next_addr + 4; 
  }
}

void getLastProg(int offset) {
  int next_addr = (2 * maxLocos) + (5 * maxLocos) + offset;
  CVPROGMODE = (int)EEPROM.read(next_addr);
  if (CVPROGMODE > CVPROGMODE_MAX) CVPROGMODE = 0;
}

bool hasData(int offset) {
  int next_addr = (2 * maxLocos) + (5 * maxLocos) + 1 + offset;
  int retVal = EEPROM.read(next_addr);
  if (retVal == 237) {
    return true;
  } else {
    EEPROM.update(next_addr, 237);
    return false;
  }
}

void saveLastProg(int offset) {
  int next_addr = (2 * maxLocos) + (5 * maxLocos) + offset;
  EEPROM.update(next_addr, CVPROGMODE);
}

void saveAddresses(int offset) {
  int lsb = 0;
  for (int i = 0; i < maxLocos; i++) {
    lsb = LocoAddress[i] / 256;
    #if debug == 1
      Serial.println(" ");
      Serial.print("loco = ");
      Serial.print(LocoAddress[i]);
      Serial.print("  address# = ");
      Serial.print(i);
      Serial.print(" msb ");
      Serial.print(lsb);
      Serial.print(" writing to ");
      Serial.print(i * 2);
      Serial.print(" and ");
      Serial.print(i * 2 + 1);
    #endif
    EEPROM.update((i * 2) + offset, lsb);
    lsb = LocoAddress[i] - (lsb * 256);
    #if debug == 1
      Serial.print(" lsb ");
      Serial.print(lsb);
    #endif
    EEPROM.update((i * 2 + 1) + offset, lsb);
  }
}

void savePushFunctions(int offset) {
  // save PushFunctions
  int next_addr = (2 * maxLocos) + offset;
  for (int i = 0; i < maxLocos; i++) {
      EEPROM.update(next_addr, (LocoPushFunction[i] >> 24) & 0xFF);
      EEPROM.update(next_addr + 1, (LocoPushFunction[i] >> 16) & 0xFF);
      EEPROM.update(next_addr + 2, (LocoPushFunction[i] >> 8) & 0xFF);
      EEPROM.update(next_addr + 3, LocoPushFunction[i] & 0xFF);
      next_addr = next_addr + 4;
  }
}
