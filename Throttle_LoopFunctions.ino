
void loop_loco_key() {

  switch (key) {
    case '*':
      if (active_modus == 1) {
        // prog mode
        // sets or delete complete push function
        if (LocoPushFunction[ActiveAddress] == 0) LocoPushFunction[ActiveAddress] = 0x1FFFFFFF;
        else LocoPushFunction[ActiveAddress] = 0;
        InitialiseFunctionLCD();
      } else {
        // if LocoSpeed > 0 set first speed to zero: Emergency Stop
        // converts Speed how JMRI works 
        if (LocoSpeed[ActiveAddress] > 0) {
          LocoSpeed[ActiveAddress] = -1;
          doDCCspeed(ActiveAddress);
          LocoSpeed[ActiveAddress] = 0;
          re_absolute = LocoSpeed[ActiveAddress];
          LocoZeroCount[ActiveAddress] = 0;
          //doDCCspeed(ActiveAddress);
          updateSpeedsLCD(ActiveAddress);
        } else {
          InitialiseSpeedsLCD();
          getLocoAddress();
          updateSpeedsLCD(ActiveAddress);
          key = 0;
        }
      }
      key = 0;
      break;
    case 'P': //Ent
      track_power = !track_power;
      if (track_power) {
        mySerial.print("<1 MAIN>");
      } else {
        mySerial.print("<0 MAIN>");
      }
      UpdateFunctionLCD(29);
      key = 0;
      break;
    case 'U':
      // Loco Up
      ActiveAddress--;
      if (ActiveAddress < 0) ActiveAddress = maxLocos - 1;
      updateSpeedsLCD(ActiveAddress);
      InitialiseFunctionLCD();
      key = 0;
      re_absolute = LocoSpeed[ActiveAddress];
      doDCCspeed(ActiveAddress);
      break;
    case 'D':
      // Loco Down
      ActiveAddress++;
      if (ActiveAddress >= maxLocos) ActiveAddress = 0;
      updateSpeedsLCD(ActiveAddress);
      InitialiseFunctionLCD();
      key = 0;
      re_absolute = LocoSpeed[ActiveAddress];
      doDCCspeed(ActiveAddress);
      break;    
    case 'E': //ESC
      // Toggle Mode to CV Prog
      CVPROG = true;
      lastUpdateCVModus = 0;
      UpdateProgLCD(true);    // um beim hin und herschalten im richtigen modus zu landen
      if (CVPROGMODE < 2) {
        // normal Prog or POM
        updateCVLCD(CVNumber, 1);
        updateCVLCD(CVVal, 2);
      } else if (CVPROGMODE == 2) {
        // Decoder Addr
        updateCVLCD(DecoderAddr, 1);
        updateCVAddrLCD(0);
        // ToDo
      } else if (CVPROGMODE == 3) {
        
      }
      updateCVHexLCD();
      updateCVBinLCD();
      UpdateCVModusLCD(true);
      key = 0;
      break;
    case 'A': //F1
      doExtendedFunction(1,1);
      updateSpeedsLCD(ActiveAddress);
      key = 0;
      break;
    case 'B': //F2
      doExtendedFunction(1,2);
      updateSpeedsLCD(ActiveAddress);
      key = 0;
      break;
    case '#': //#
      //Set All Functions on Active Address to Zero
      if (active_modus == 1) {
        // Programming
        active_modus = last_modus;
        savePushFunctions(1);
        UpdateModusLCD();
        break;
      }
      if ((LocoFunction[ActiveAddress] == 0) && (active_modus != 1)) {
        last_modus = active_modus;
        active_modus = 1;
        UpdateModusLCD();
        break;
      }
      doFunction(29);
      InitialiseFunctionLCD();
      break;
    case 'L': //direction Left / Back
      if (LocoSpeed[ActiveAddress] == 0) {
        LocoDirection[ActiveAddress] = 0;
        LocoZeroCount[ActiveAddress] = 0;
        doDCCspeed(ActiveAddress);
        updateSpeedsLCD(ActiveAddress);
      }
      break;
    case 'R': //direction Right / Forward
      if (LocoSpeed[ActiveAddress] == 0) {
        LocoDirection[ActiveAddress] = 1;
        LocoZeroCount[ActiveAddress] = 0;
        doDCCspeed(ActiveAddress);
        updateSpeedsLCD(ActiveAddress);
      }    
      break;
    default:
      // It's 0 - 9 or A - C so perform a loco function
      key = key - 48;
      //if (key > 10) key = key - 7;
      if (key > 10) break;
      doFunction(key);
      break;
  }
}

void loop_prog_key() {

  // CVPROG == true
  switch (key) {
    case '*':
      // Set PROG Mode
      CVPROGMODE++;
      if (CVPROGMODE > CVPROGMODE_MAX) CVPROGMODE = 0;
      UpdateProgLCD(true);
      if (CVPROGMODE < 2) {
        // normal Prog or POM

        CVOne = CVNumberArr[CVCol - 5] - 48; // Addr
        if ((CVLine == 2) && (CVCol > 4)) CVOne = CVValArr[CVCol - 5] - 48; // Value
        if ((CVLine == 3) && (CVCol > 4)) CVOne = bitRead(CVVal, 12 - CVCol); // binary

        updateCVLCD(CVNumber, 1);
        updateCVLCD(CVVal, 2);
      } else if (CVPROGMODE == 2) {
        // Decoder Addr

        CVOne = DecoderAddrArr[CVCol - 5] - 48;
        
        updateCVLCD(DecoderAddr, 1);
        if (CVLine > 1) CVCol = 4;
        updateCVAddrLCD(0);
        // ToDo
      } else if (CVPROGMODE == 3) {
        
      }
      saveLastProg(1);
      updateCVHexLCD();
      updateCVBinLCD();
      UpdateCVModusLCD(true);
      break;
      
    case 'P': //Ent
      track_power = !track_power;
      if (track_power) {
        mySerial.print("<1 MAIN>");
      } else {
        mySerial.print("<0 MAIN>");
      }
      UpdateCVModusLCD(false);
      key = 0;
      break;
      
    case 'U': // Up
      if (CVPROGMODE == 3) break;
      CVLine--;
      if (CVLine < 1) CVLine = 1;
      CVCol = 4;
      lcd.setCursor(CVCol, CVLine);
      break;
      
    case 'D': // Down
      if (CVPROGMODE == 3) break;
      CVLine++;
      if (CVLine > 3) CVLine = 3;
      CVCol = 4;
      lcd.setCursor(CVCol, CVLine);
      break;
      
    case 'E': //ESC
      // Toggle Mode to Normal
      CVPROG = false;
      lcd.clear();
      InitialiseSpeedsLCD();
      InitialiseFunctionLCD();
      key = 0;
      break;
      
    case 'A': //F1
      if (CVPROGMODE == 0) {
        updateCVMsg("Read");
        mySerial.print("<R ");
        mySerial.print(CVNumber);
        mySerial.print(" ");
        mySerial.print(CB_NUM);
        mySerial.print(" ");
        mySerial.print(CB_READ_BYTE);
        mySerial.print(">");
        lastCV = millis();
        lastDatas = DataTimeout + millis(); // dont get other datas
        CVSend = 1;
      } else if (CVPROGMODE == 2) {
        // LocoAddr
        GetDecoderAddr(0,0);
      } else if (CVPROGMODE == 3) {
        //Decoder Info
        GetDecoderInfo(0,0);
      }
      break;
      
    case 'B': //F2
      if (CVPROGMODE == 3) break;
      updateCVMsg("Write");
      if (CVPROGMODE == 0) {
        mySerial.print("<W ");
        mySerial.print(CVNumber);
        mySerial.print(" ");
        mySerial.print(CVVal);
        mySerial.print(" ");
        mySerial.print(CB_NUM);
        mySerial.print(" ");
        mySerial.print(CB_WRITE_BYTE);
        mySerial.print(">");
        CVSend = 1;
        lastCV = millis();
        lastDatas = DataTimeout + millis(); // dont get other datas
      } else if (CVPROGMODE == 1) {
        // < w CAB CV VALUE >
        mySerial.print("<w ");
        mySerial.print(LocoAddress[ActiveAddress]);
        mySerial.print(" ");
        mySerial.print(CVNumber);
        mySerial.print(" ");
        mySerial.print(CVVal);
        mySerial.print(">");
        CVSend = 2;
        lastCV = millis();
        lastDatas = DataTimeout + millis(); // dont get other datas
      } else if (CVPROGMODE == 2) {
        // LocoAddr
        SetDecoderAddr(0,0);
      }
      break;
      
    case '#': //#
      if (CVPROGMODE == 3) break;
      break;
      
    case 'L': //direction Left
      if (CVPROGMODE == 3) break;
      CVCol--;
      // if (CVCol < 4) CVCol = 4;
      if ((CVLine == 1) && (CVCol < 4)) CVCol = 8; //Addr
      if ((CVLine == 1) && (CVCol > 4)) {
        if (CVPROGMODE == 2) {
          // Decoder Addr
          CVOne = DecoderAddrArr[CVCol - 5] - 48;
        } else {
          CVOne = CVNumberArr[CVCol - 5] - 48; // Addr
        }
      }
      if (CVPROGMODE < 2) {
        // normal Prog or POM
        if ((CVLine == 2) && (CVCol < 4)) CVCol = 7; //Value
        if ((CVLine == 3) && (CVCol < 4)) CVCol = 12; //Bin
        if ((CVLine == 2) && (CVCol > 4)) CVOne = CVValArr[CVCol - 5] - 48; // Addr or Value
        if ((CVLine == 3) && (CVCol > 4)) CVOne = bitRead(CVVal, 12 - CVCol); // binary
      } else if (CVPROGMODE == 2) {
        // Decoder Addr
        if (CVLine > 1 && CVCol != 4) CVCol = 4;
      }
      lcd.setCursor(CVCol, CVLine);
      break;
      
    case 'R': //direction Right
      if (CVPROGMODE == 3) break;
      CVCol++;
      if ((CVLine == 1) && (CVCol > 8)) CVCol = 4; //Addr
      if ((CVLine == 1) && (CVCol > 4)) CVOne = CVNumberArr[CVCol - 5] - 48; // Addr

      if (CVPROGMODE == 2) {
        // Decoder Addr
        CVOne = DecoderAddrArr[CVCol - 5] - 48;
      } else {
        CVOne = CVNumberArr[CVCol - 5] - 48; // Addr
      }
      
      if (CVPROGMODE < 2) {
        // normal Prog or POM
        if ((CVLine == 2) && (CVCol > 7)) CVCol = 4; //Value
        if ((CVLine == 3) && (CVCol > 12)) CVCol = 4; //Bin
        if ((CVLine == 2) && (CVCol > 4)) CVOne = CVValArr[CVCol - 5] - 48; // Value
        if ((CVLine == 3) && (CVCol > 4)) CVOne = bitRead(CVVal, 12 - CVCol); // binary
      } else if (CVPROGMODE == 2) {
        // Decoder Addr
        if (CVLine > 1 && CVCol != 4) CVCol = 4;
      }
      lcd.setCursor(CVCol, CVLine);
      break;
      
    default:
      if (CVPROGMODE == 3) break;
      // It's 0 - 9
      key = key - 48;
      if (key > 10) break;
      if (CVCol == 4) CVCol = 5;
      lcd.setCursor(CVCol, CVLine);
      CVOne = key;
      if (CVLine == 1) {
        if (CVPROGMODE == 2) {
          // Decoder Addr
          if (!DecoderLong) {
            if (CVCol < 7) CVOne = 0;
            if (CVCol == 8 && CVOne == 0) CVOne = 1;
          } else {
            DecoderAddrArr[CVCol - 5] = CVOne + 48;
            DecoderAddr = atoi(DecoderAddrArr);
            if (DecoderAddr > 9999) DecoderAddr = 0;
            if (DecoderAddr < 128) {
              DecoderAddr = 128;
              updateCVLCD(DecoderAddr, CVLine);
              //String(DecoderAddr).toCharArray(DecoderAddrArr, sizeof(DecoderAddrArr));
              strncpy(DecoderAddrArr, "0128", 4);
              CVOne = DecoderAddrArr[CVCol - 5] - 48; // Value
              
            }
          }
        }
        SetCVOne(CVOne);
      }
      if (CVPROGMODE < 2) {
        // normal Prog or POM
        if (CVLine == 2) {
          CVOne = CheckCVValue(CVOne, true);
        }
        if (CVLine == 3) {
          if (CVOne > 1) CVOne = 1;
          SetCVOne(CVOne);
          updateCVLCD(CVVal, 2);
          updateCVHexLCD();
        }
      }
      CVCol++;
      if ((CVLine == 1) && (CVCol > 8)) CVCol = 4; //Addr
      if ((CVLine == 1) && (CVCol > 4)) {
        if (CVPROGMODE == 2) {
          // Decoder Addr
          CVOne = DecoderAddrArr[CVCol - 5] - 48;
        } else {
          CVOne = CVNumberArr[CVCol - 5] - 48; // Addr
        }
      }
      if (CVPROGMODE < 2) {
        // normal Prog or POM
        if ((CVLine == 2) && (CVCol > 7)) CVCol = 4; //Value
        if ((CVLine == 3) && (CVCol > 12)) CVCol = 4; //Bin
        if ((CVLine == 2) && (CVCol > 4)) CVOne = CVValArr[CVCol - 5] - 48; // Value
        if ((CVLine == 3) && (CVCol > 4)) CVOne = bitRead(CVVal, 12 - CVCol); // binary
      } else if (CVPROGMODE == 2) {
        // Decoder Addr
        if (CVLine > 1) CVCol = 4;
      }
      
      lcd.setCursor(CVCol, CVLine);
      break;
  }
    
}

void loop_rotary_prog(int8_t re_val) {

  if (CVPROGMODE < 3) {
    //CV Prog
    switch (CVLine) {
      case 1: // Address
        if (CVCol == 4) {
          if (CVPROGMODE < 2) {
            // normal Prog or POM
            CVNumber += re_val;
            if (CVNumber > 9999) CVNumber = 0;
            if (CVNumber < 0) CVNumber = 9999;
            // converts into array
            String(CVNumber).toCharArray(CVNumberArr, sizeof(CVNumberArr));
            updateCVLCD(CVNumber, CVLine);
          } else if (CVPROGMODE == 2) {
            // Decoder Addr
            DecoderAddr += re_val;
            if (DecoderLong) {
              if (DecoderAddr > 9999) {
                DecoderAddr = 128;
                strncpy(DecoderAddrArr, "0128", 4);
              }
              if (DecoderAddr < 128) {
                DecoderAddr = 9999;
                strncpy(DecoderAddrArr, "9999", 4);
              }
            } else {
              if (DecoderAddr > 99) {
                DecoderAddr = 1;
                strncpy(DecoderAddrArr, "0001", 4);
              }
              if (DecoderAddr < 1) {
                DecoderAddr = 99;
                strncpy(DecoderAddrArr, "0099", 4);
              }
            }
            // converts into array
            //String(DecoderAddr).toCharArray(DecoderAddrArr, sizeof(DecoderAddrArr));
            updateCVLCD(DecoderAddr, CVLine);
          }
        } else {
          CVOne += re_val;
          if (CVOne > 9) CVOne = 0;
          if (CVOne < 0) CVOne = 9;
          
          if (CVPROGMODE == 2) {
            // Decoder Addr
            if (!DecoderLong) {
              if (CVCol < 7) CVOne = 0;
              if (CVCol == 8 && CVOne == 0) CVOne = 1;
            } else {
              DecoderAddrArr[CVCol - 5] = CVOne + 48;
              DecoderAddr = atoi(DecoderAddrArr);
              if (DecoderAddr > 9999) DecoderAddr = 0;
              if (DecoderAddr < 128) {
                DecoderAddr = 128;
                updateCVLCD(DecoderAddr, CVLine);
                //String(DecoderAddr).toCharArray(DecoderAddrArr, sizeof(DecoderAddrArr));
                strncpy(DecoderAddrArr, "0128", 4);
                CVOne = DecoderAddrArr[CVCol - 5] - 48; // Value
                
              }
            }
            
          }
          SetCVOne(CVOne);
        }
        break;
    
      case 2: // Val
        if (CVPROGMODE < 2) {
          // normal Prog or POM
          if (CVCol == 4) {
            CVVal += re_val;
            if (CVVal > 255) CVVal = 0;
            if (CVVal < 0) CVVal = 255;
            // converts into array
            String(CVVal).toCharArray(CVValArr, sizeof(CVValArr));
            updateCVLCD(CVVal, CVLine);
            updateCVHexLCD();
            updateCVBinLCD();
          } else {
            CVOne += re_val;
            CVOne = CheckCVValue(CVOne,false);
          }
        } else if (CVPROGMODE == 2) {
          // Decoder Addr
          DecoderLong = !DecoderLong;
          if (!DecoderLong) {
            if (DecoderAddr > 99) {
              DecoderAddr = 99;
              updateCVLCD(DecoderAddr, 1);
            }
          } else {
            if (DecoderAddr < 128) {
              DecoderAddr = 128;
              updateCVLCD(DecoderAddr, 1);
            }
          }
          updateCVAddrLCD(2);
        }
        break;
    
      case 3: //bin
        if (CVPROGMODE < 2) {
          // normal Prog or POM
          if (CVCol == 4) {
            CVVal += re_val;
            if (CVVal > 255) CVVal = 0;
            if (CVVal < 0) CVVal = 255;
            // converts into array
            String(CVVal).toCharArray(CVValArr, sizeof(CVValArr));
            updateCVLCD(CVVal, 2);
            updateCVHexLCD();
            updateCVBinLCD();
          } else {
            CVOne += re_val;
            if (CVOne > 1) CVOne = 0;
            if (CVOne < 0) CVOne = 1;
            SetCVOne(CVOne);
            updateCVLCD(CVVal, 2);
            updateCVHexLCD();
          }
        } else if (CVPROGMODE == 2) {
          // Decoder Addr
          DecoderSpeedStep = !DecoderSpeedStep;
          updateCVAddrLCD(3);
        }
        break;
        
    }
  
  }
  
}
