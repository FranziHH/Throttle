
int CheckCVValue(int value, bool iskey) {

  if (value > 0) {
    CVValArr[CVCol - 5] = value + 48;
    CVVal = atoi(CVValArr);
  }
   
  switch (CVCol) {
      case 5: // 0..2
        if (iskey) {
          if (value > 2) value = 2;
        } else {
          if (value > 2) value = 0;
        }
        if (value < 0) value = 2;
        break;

      case 6:
        if (CVVal > 199) { // 0..5
          if (iskey) {
            if (value > 5) value = 5;
          } else {
            if (value > 5) value = 0;
          }
          if (value < 0) value = 5;
        } else { // 0..9
          if (value > 9) value = 0;
          if (value < 0) value = 9;
        }
        break;
        
      case 7:
        if (CVVal > 249) { // 0..5
          if (iskey) {
            if (value > 5) value = 5;
          } else {
            if (value > 5) value = 0;
          }
          if (value < 0) value = 5;
        } else { // 0..9
          if (value > 9) value = 0;
          if (value < 0) value = 9;
        }
        break;
      
    }

    CVValArr[CVCol - 5] = value + 48;
    CVVal = atoi(CVValArr);
    
    if (CVVal > 255) {
      CVVal = 255;
      String(CVVal).toCharArray(CVValArr, sizeof(CVValArr));
      updateCVLCD(CVVal, CVLine);
      switch (CVCol) {
        case 5:
          value = 2;
          break;

        case 6:
          value = 5;
          break;
          
        case 7:
          value = 5;
          break;
      }

      updateCVLCD(CVVal, CVLine);
    } else {
      SetCVOne(value);
    }

    updateCVHexLCD();
    updateCVBinLCD();

    return value;
}

//START DO FUNCTION BUTTONS
void doExtendedFunction(int counter, int newnumber) {
  
  int total = 0;
  if (counter == 0) {
    lcd.setCursor(9 , 0);
  } else {
    lcd.setCursor(9 , newnumber + 1);
    total = newnumber * 10;    
  }
  do {
    if (millis() - lastKeyPush  > PushKeyTimeout) {
      key = keypad.getKey();
      if (key) {
        counter++;
        // Abort if # or *
        if (key < 48) return;
        // otherwise...
        int number =  key - 48;
        // if it 3-9 or A-D, and this is the first key...
        if (number > 2 && counter == 1) {
          #if debug == 1
            Serial.print("First Time, 3 to D");
          #endif
          return;
        }
        // else we can assume it's 0,1,2...
        else if ( counter == 1 ) {
          lcd.setCursor(9 , number + 1);
          total = number * 10;
        }
        else if (counter == 2 && number < 10) {
          // Second time around... and 0-9
          lcd.setCursor(number + 10 , total / 10 + 1);
          total = total + number;
        }
        else if (counter == 2 && number > 9) {
          #if debug == 1
            Serial.print("Second Time, A-D");
          #endif
          return;
        }
      }
      lastKeyPush = millis();
    }
  } while (counter <= 1); //  collect exactly 2 digits
  #if debug == 1
    Serial.print(total);
  #endif

  if (total < 29) {
    // at direct access don't use 29 for delete
    doFunction(total);
  }
}

void doFunction(int fN) {
  // Will be passed a number from 0 to 28.
  if (fN == 29) {
    LocoFunction[ActiveAddress] = 0;
    doDCCfunctionReset(ActiveAddress);
  } else {
    if (active_modus == 1) {
      // Prog Modus
      bitWrite(LocoPushFunction[ActiveAddress], fN, !bitRead(LocoPushFunction[ActiveAddress], fN));
    } else {
      if (bitRead(LocoPushFunction[ActiveAddress], fN) == 1) {
        // if is pushfunction add to PushOff
        if (bitRead(LocoFunction[ActiveAddress], fN) == 0) {
          bitWrite(LocoFunction[ActiveAddress], fN, 1);
          // ontime, addr, function
          lastDatas = millis() + PushTimeout;
          LocoPushOff[next_push_idx][0] = millis();
          LocoPushOff[next_push_idx][1] = ActiveAddress;
          LocoPushOff[next_push_idx][2] = fN;
          next_push_idx++;
          if (next_push_idx > 5) next_push_idx = 0;
          doDCCfunction(fN, ActiveAddress);
        }
      } else {
        bitWrite(LocoFunction[ActiveAddress], fN, !bitRead(LocoFunction[ActiveAddress], fN));
        doDCCfunction(fN, ActiveAddress);
      }
    }
  }
  UpdateFunctionLCD(fN);
}

void getLocoAddress() {
  int saveAddress = LocoAddress[ActiveAddress];
  int total = 0;
  int counter = 0;
  do {
    if (millis() - lastKeyPush  > PushKeyTimeout) {
      lcd.setCursor( counter , ActiveAddress);
      key = keypad.getKey();
      if (key == '#' || key == '*' || key == 'A' || key == 'B' || key == 'U' || key == 'D' || key == 'E' || key == 'P' || key == 'R' || key == 'L') { //abort when either is hit
        //LocoAddress[ActiveAddress] = saveAddress;
        total = saveAddress;
        break;// exits the do...while loop if above buttons pressed - ABORT new address
      }
      if (key) {
        counter++;
        int number = key - 48;
        total = total * 10 + number;
        if (key == 48 && total == 0) {
          lcd.print(" ");
        } else {
          lcd.print(key);
        }
        #if debug == 1
          Serial.print("Counter = ");
          Serial.print(counter);
          Serial.print("  key = ");
          Serial.print(key);
          Serial.print("   val = ");
          Serial.println(number);
        #endif
      }
      lastKeyPush = millis();
    }
  } while (counter <= 3); //  collect exactly 4 digits
  //  lcd.noBlink();
  // If all zeroes entered, return to original address (DCC++ doesn't handle 0.)
  if (total == 0) total = saveAddress;
  LocoAddress[ActiveAddress] = total;
  #if debug == 1
    Serial.print("Actually saving: ");
    Serial.println(total);
  #endif
  saveAddresses(1);
  updateSpeedsLCD(ActiveAddress);
}

void doDCCspeed(int addr) {
  #if debug == 1
    Serial.println(LocoDirection[addr] );
  #endif
  mySerial.print("<t1 ");
  mySerial.print(LocoAddress[addr] );//locoID);
  mySerial.print(" ");
  mySerial.print(LocoSpeed[addr] );
  mySerial.print(" ");
  mySerial.print(LocoDirection[addr] );
  mySerial.println(">");
  LocoInUse[addr] = true;
  lastDatas = millis();
}

void doDCCfunction(int fN, int addr) {
  mySerial.write("<F ");
  mySerial.print(LocoAddress[addr]);
  mySerial.print(" ");
  mySerial.print(fN);
  mySerial.print(" ");
  mySerial.print(bitRead(LocoFunction[addr], fN));
  mySerial.print(">");
  LocoInUse[addr] = true;
  lastDatas = millis();
}

void doDCCfunctionReset(int addr) {
  mySerial.write("<F ");
  mySerial.print(LocoAddress[addr]);
  mySerial.print(">");
  LocoInUse[addr] = true;
  lastDatas = millis();
}

void all2ZeroSpeed() {
  /* Loads of bugs here.
      A) tempx <= maxLocos meant five commands were sent, the fifth to a random(?) loco.
      B) LocoSpeed and Direction were set to those of loco 1.  Not good practice, although not required to be correct as <0> is sent after.
      As of 4thAugust2020, modified to do what it says it does.
  */
  for (int tempx = 0; tempx < maxLocos; tempx++) {
    // Set the recorded speeds to zero...
    LocoSpeed[tempx] = 0;
    // ... then transmit the commands too.
    mySerial.print("<t1 ");
    mySerial.print(LocoAddress[tempx] );//locoID);
    mySerial.print(" 0 ");
    mySerial.print(LocoDirection[tempx] );
    mySerial.write(">");
  }
  lastDatas = millis();
}

void GetLocoDatas() {
  /* Gets Data from ussed locos */
  // Format is: <G CV BIT VALUE CALLBACKNUM CALLBACKSUB>
  mySerial.print("<G");
  for (int tempx = 0; tempx < maxLocos; tempx++) {
    if (LocoInUse[tempx]) {
      mySerial.print(" ");
      mySerial.print(LocoAddress[tempx]);
    }
  }
  mySerial.print(" ");
  mySerial.print(DEVICE_ID);
  mySerial.print(" ");
  mySerial.print(CB_NUM_GETLOCODATA);
  mySerial.write(">");
  cmd_count++;
}
