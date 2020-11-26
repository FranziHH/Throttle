
void SetCVOne(int value) {
    // converts into array
    switch (CVLine) {
      case 1: // Address
        if (CVPROGMODE == 2) {
          DecoderAddrArr[CVCol - 5] = value + 48;
          DecoderAddr = atoi(DecoderAddrArr);
        } else {
          CVNumberArr[CVCol - 5] = value + 48;
          CVNumber = atoi(CVNumberArr);
        }
        lcd.setCursor(CVCol, CVLine);
        lcd.print(value);
        lcd.setCursor(CVCol, CVLine);
        break;

      case 2: // Value
        CVValArr[CVCol - 5] = value + 48;
        CVVal = atoi(CVValArr);
        lcd.setCursor(CVCol, CVLine);
        lcd.print(value);
        lcd.setCursor(CVCol, CVLine);
        break;

      case 3: // binary
        bitWrite(CVVal, 12 - CVCol, value);
        String(CVVal).toCharArray(CVValArr, sizeof(CVValArr) + 1);
        lcd.print(value);
        lcd.setCursor(CVCol, CVLine);
        break;

    }

}

void updateCVMsg(char msg[]) {
  // Print ACK Message
  lcd.setCursor(14, 3);
  lcd.print("      ");
  lcd.setCursor(14, 3);
  lcd.print(msg);
  if (CVPROGMODE == 3 && CVPROG) {
    lcd.setCursor(13, 0);
  } else {
    lcd.setCursor(CVCol, CVLine);
  }
}

void updateCVHexLCD() {
  // Print HEX Value
  if (CVPROGMODE < 2) {
    lcd.setCursor(18, 2);
    if (CVVal < 16) lcd.print("0");
    lcd.print(CVVal, HEX);
    lcd.setCursor(CVCol, CVLine);
  }
}

void updateCVBinLCD() {
  if (CVPROGMODE < 2) {
    char cFunc[9];
    for (int i = 0; i < 8; i++) cFunc[7 - i] = bitRead(CVVal, i) + 48;
    cFunc[8] = '\0';
    lcd.setCursor(5, 3);
    lcd.print(cFunc);
    lcd.setCursor(CVCol, CVLine);
  }
}

void updateCVAddrLCD(int line) {
  // CVPROGMODE == 2 -> Decoder Get/Set Addr

  if (line == 0 || line == 2) {
    lcd.setCursor(5, 2);
    if (DecoderLong) lcd.print("long ");
    else lcd.print("short");
    if (line > 0) lcd.setCursor(4, line);
  }

  if (line == 0 || line == 3) {
    lcd.setCursor(5, 3);
    if (DecoderSpeedStep) lcd.print("28/126");
    else lcd.print("14    ");
    if (line > 0) lcd.setCursor(4, line);
  }
}

void updateCVLCD(int value, int line) {
    switch (line) {
      case 1: // Address
        lcd.setCursor(5, line);
        if (value < 10) lcd.print("0");
        if (value < 100) lcd.print("0");
        if (value < 1000) lcd.print("0");
        lcd.print(value);
        lcd.setCursor(4, line);
        break;

      case 2: // value
        lcd.setCursor(5, line);
        if (value < 10) lcd.print("0");
        if (value < 100) lcd.print("0");
        lcd.print(value);
        lcd.setCursor(4, line);
        break;

      case 3: //binary
        break;

    }
}

void InitialiseProgLCD() {
  switch (CVPROGMODE) {
    case 0: // Prog Mode "normal"
    case 1: // POM
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("CV : 0000");
      lcd.setCursor(0, 2);
      lcd.print("Val: 000      Hex:00");
      lcd.setCursor(0, 3);
      lcd.print("     00000000");
      break;

    case 2: //Addr
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Adr: 0000");
      break;

    case 3: //Info
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Decoder Info F1:Read");
      lcd.setCursor(0, 1);
      lcd.print("M:");
      lcd.setCursor(0, 2);
      lcd.print("T:");
      lcd.setCursor(0, 3);
      lcd.print("V:");
      break;
  }
}

void UpdateProgLCD(bool renew) {
  if (renew) InitialiseProgLCD();
  lcd.setCursor(0, 0);

  switch (CVPROGMODE) {
    case 0: // Prog Mode "normal"
      lcd.print("PRG F1:Read F2:Write");
      lcd.setCursor(CVCol, CVLine);
      break;

    case 1: // POM
      lcd.print("POM: ");
      if (LocoAddress[ActiveAddress] < 10) lcd.print("0");
      if (LocoAddress[ActiveAddress] < 100) lcd.print("0");
      if (LocoAddress[ActiveAddress] < 1000) lcd.print("0");
      lcd.print(LocoAddress[ActiveAddress]);
      lcd.print("   F2:Write");
      lcd.setCursor(CVCol, CVLine);
      break;

    case 2: // Loco Addr
      lcd.print("ADR F1:Read F2:Write");
      lcd.setCursor(CVCol, CVLine);
      break;

    case 3: // Loco Info
      lcd.setCursor(13, 0);
      break;
  }

}

void InitialiseSpeedsLCD() {
  for (int tempx = 0; tempx < maxLocos; tempx++) {
    // Prints LocoID(right justified), direction arrow, and speed(right justified)
    lcd.setCursor(0, tempx);
    String temp = "   " + String(LocoAddress[tempx] , DEC);
    int tlen = temp.length() - 4;
    lcd.print(temp.substring(tlen));
    // ... direction...
    if (LocoDirection[tempx] == 1 ) {
      lcd.print(">");
    }
    else {
      lcd.print("<");
    }
    // ... speed ...
    temp = "  " + String(LocoSpeed[tempx] , DEC);
    tlen = temp.length() - 3;
    lcd.print(temp.substring(tlen));
  }
  // Return cursor to direction arrow for loco under control
  lcd.setCursor(4, ActiveAddress);
}

void InitialiseFunctionLCD() {
  int n = 0;
  lcd.setCursor(9, 0);
  // 0 - normal, 1 - Prog Functions, 2 - no connection
  if (active_modus == 0) lcd.print("F");
  if (active_modus == 1) lcd.print("P");
  if (active_modus == 2) lcd.print("X");
  lcd.print("0123456789");
  for (int y = 0; y < 3; y++) {
    lcd.setCursor(9, y + 1);
    lcd.print(y);
    for (int x = 0; x < 10; x++) {
      lcd.setCursor(x + 10, y + 1);
      n = y * 10 + x;
      if (n < 29) {
        lcd.write((bitRead(LocoFunction[ActiveAddress], n) == 0) ? (bitRead(LocoPushFunction[ActiveAddress], n) == 0) ? byte(0) :  byte(3) : byte(1));
        //lcd.write(byte(bitRead(LocoFunction[ActiveAddress], n)));
      } else {
        // 29th location, hint that 29 will turn off all loco functions.
        if (track_power) {
          //lcd.print("X");
          lcd.write(byte(2));
        } else {
          lcd.print("O");
        }
      }

    }
    lcd.setCursor(4, ActiveAddress);
  }
}

void UpdateFunctionLCD(int FN) {
  int y = FN / 10;
  int x = FN - (y * 10);
//  Serial.print("\nSetCursor: ");
//  Serial.print(x);
//  Serial.print(":");
//  Serial.println(y);
  lcd.setCursor(x + 10, y + 1);
  if (FN < 29) {
    lcd.write((bitRead(LocoFunction[ActiveAddress], FN) == 0) ? (bitRead(LocoPushFunction[ActiveAddress], FN) == 0) ? byte(0) :  byte(3) : byte(1));
    //lcd.write(byte(bitRead(LocoFunction[ActiveAddress], FN)));
  } else {
    // 29th location, hint that 29 will turn off all loco functions.
    if (track_power) {
      //lcd.print("X");
      lcd.write(byte(2));
    } else {
      lcd.print("O");
    }
  }
  lcd.setCursor(4, ActiveAddress);
}

void UpdateModusLCD() {
  lcd.setCursor(9, 0);
  // 0 - normal, 1 - Prog Functions, 2 - no connection
  if (active_modus == 0) lcd.print("F");
  if (active_modus == 1) lcd.print("P");
  if (active_modus == 2) lcd.print("X");
  lcd.setCursor(4, ActiveAddress);
}

void UpdateCVModusLCD(bool force) {
  if (CVPROG && CVPROGMODE == 3) return;
  if (force) lastUpdateCVModus = 0;
  if (cmd_connected) {
    if (track_power) {
      //lcd.print("X");
      UpdateCVModus = 1;
      if (UpdateCVModus != lastUpdateCVModus) {
        lcd.setCursor(0, 3);
        lcd.write(byte(2));
        lcd.setCursor(CVCol, CVLine);
        lastUpdateCVModus = UpdateCVModus;
      }
    } else {
      UpdateCVModus = 2;
      if (UpdateCVModus != lastUpdateCVModus) {
        lcd.setCursor(0, 3);
        lcd.print("O");
        lcd.setCursor(CVCol, CVLine);
        lastUpdateCVModus = UpdateCVModus;
      }
    }
  } else {
    UpdateCVModus = 3;
    if (UpdateCVModus != lastUpdateCVModus) {
      lcd.setCursor(0, 3);
      lcd.print("X");
      lcd.setCursor(CVCol, CVLine);
      lastUpdateCVModus = UpdateCVModus;
    }
  }

}

void updateSpeedsLCD(int addr) {
  lcd.setCursor(0, addr);
  String temp = "   " + String(LocoAddress[addr] , DEC);
  int tlen = temp.length() - 4;
  lcd.print(temp.substring(tlen));
  if (LocoDirection[addr] == 1 ) {
    lcd.print(">");
  }
  else {
    lcd.print("<");
  }
  temp = "  " + String(LocoSpeed[addr] , DEC);
  tlen = temp.length() - 3;
  lcd.print(temp.substring(tlen));
  lcd.setCursor(4, ActiveAddress);
}
