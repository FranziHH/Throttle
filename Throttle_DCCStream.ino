
bool GetDecoderAddr(int CVNr, int CVVal) {
  // all steps to get decoder Adress
    
    static int CV29;
    static int CV18;
    static int CV17;
    int NextNr = 0;
    
    switch(CVNr) {
      case 0:
        //Start Reading
        // ToDo: Set Display
        CV29 = 0;
        CV18 = 0;
        CV17 = 0;
        NextNr = 29;
        break;
        
      case 29:
        // Get Infos Short/Long
        CV29 = CVVal;
        if (bitRead(CVVal, 5) == 1) {
          // long Address
          DecoderLong = true;
          NextNr = 17;
        } else {
          // short Address
          DecoderLong = false;
          NextNr = 1;
        }
        if (bitRead(CVVal, 1) == 1) {
          // SpeedSteps
          DecoderSpeedStep = true;
        } else {
          DecoderSpeedStep = false;
        }
        break;

      case 17:
        CV17 = CVVal;
        NextNr = 18;
        break;

      case 18:
        CV18 = CVVal;
        DecoderAddr = ((CV17 & 0x3F) << 8) + CV18;
        NextNr = 0;
        break;

      case 1:
        DecoderAddr = CVVal;
        NextNr = 0;
        break;
        
    }

    if (NextNr > 0) {
      int tmp_CB_VAR = CB_NUM_GETADDR;
      bitWrite(tmp_CB_VAR, CB_READ_BYTE, 1);
      updateCVMsg("Read");
      mySerial.print("<R ");
      mySerial.print(NextNr);
      mySerial.print(" ");
      mySerial.print(DEVICE_ID);
      mySerial.print(" ");
      mySerial.print(tmp_CB_VAR);
      mySerial.print(">");
      lastCV = millis();
      lastDatas = DataTimeout + millis(); // dont get other datas
      CVSend = 1;
      return false;
    } else {
      // ToDo: 
      updateCVLCD(DecoderAddr, 1);
      updateCVAddrLCD(0);
      updateCVMsg("R:Ok");
      CVSend++;
      return true;
    }
    
}

bool SetDecoderAddr(int CVNr, int CVVal) {
  // all steps to get decoder Adress
    
    int NextNr = 0;
    bool flgWrite = false;
    
    switch(CVNr) {
      case 0:
        //Start Writing
        // ToDo: Set Display
        flgWrite = false;
        NextNr = 29;
        break;
        
      case 290: // from read ....
        // Set Infos Short/Long
        bitWrite(CVVal, 5, DecoderLong ? 1 : 0);
        // Set SpeedSteps
        bitWrite(CVVal, 1, DecoderSpeedStep ? 1 : 0);
        flgWrite = true;
        NextNr = 29;
        break;

      case 29: // from write ....
        if (!DecoderLong) {
          CVVal = DecoderAddr;
          flgWrite = true;
          NextNr = 1;
        } else {
          int CV18 = DecoderAddr & 0xFF;
          int CV17 = ((DecoderAddr - CV18) >> 8) | 0xC0;
          CVVal = CV17;
          flgWrite = true;
          NextNr = 17;
        }
        break;
        
      case 17:
        int CV18 = DecoderAddr & 0xFF;
        int CV17 = ((DecoderAddr - CV18) >> 8) | 0xC0;
        CVVal = CV18;
        flgWrite = true;
        NextNr = 18;
        break;

      case 18:
        NextNr = 0;
        break;

      case 1:
        NextNr = 0;
        break;
        
    }

    if (NextNr > 0) {
      int tmp_CB_VAR = CB_NUM_SETADDR;
      if (!flgWrite) {
        updateCVMsg("Read");
        mySerial.print("<R ");
      } else {
        updateCVMsg("Write");
        mySerial.print("<W ");
      }
      mySerial.print(NextNr);
      mySerial.print(" ");
      if (flgWrite) {
        mySerial.print(CVVal);
        mySerial.print(" ");
      }
      mySerial.print(DEVICE_ID);
      mySerial.print(" ");
      if (!flgWrite) bitWrite(tmp_CB_VAR, CB_READ_BYTE, 1);
      else bitWrite(tmp_CB_VAR, CB_WRITE_BYTE, 1);
      mySerial.print(tmp_CB_VAR);
      mySerial.print(">");
      lastCV = millis();
      lastDatas = DataTimeout + millis(); // dont get other datas
      CVSend = 1;
      return false;
    } else {
      // ToDo: 
      updateCVMsg("W:Ok");
      CVSend++;
      return true;
    }
    
}

#if (defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560))
bool GetDecoderInfo(int CVNr, int CVVal) {
  // all steps to get decoder Infos
    
    int NextNr = 0;
    bool flgWrite = false;
    
    if (CVNr == 0) DecoderValue = 0;
    
    if (DecoderValue == 151) {
      // is ESU
      
      // http://www.esu.eu/support/faq/lokpilot/lokpilot-v40/
      switch(CVNr) {
        case 31:
          NextNr = 32;
          CVVal = 255;
          flgWrite = true;
          break;

        case 32:
          NextNr = 288;
          break;
        
        case 288:
          ValVersion = CVVal;  
          NextNr = 287;
          break;

        case 287:
          ValSubversion = CVVal;  
          NextNr = 286;
          break;
          
        case 286:
          ValBuild = CVVal * 256;  
          NextNr = 285;
          break;
          
        case 285:
          ValBuild += CVVal;
          NextNr = 264;
          break;

        case 264:
          CVType = CVVal * 16777216;
          NextNr = 263;
          break;

        case 263:
          CVType += CVVal * 65536;
          NextNr = 262;
          break;

        case 262:
          CVType += CVVal * 256;
          NextNr = 261;
          break;

        case 261:
          CVType += CVVal;
          lcd.setCursor(2, 2);
          String("ESU.csv").toCharArray(ExtFile, sizeof(ExtFile));
          int retVal = GetDecoderTyp(CVType, true);
          switch(retVal) {
            case 0:
              //SD Card not found
              lcd.print(CVType, HEX);
              lcd.print(" - no Card");
              break;
              
            case 1:
              //CSV File not Found
              lcd.print(CVType, HEX);
              lcd.print(" - no File");
              break;
              
            case 2:
              //Decoder not found
              lcd.print(CVType, HEX);
              lcd.print(" - no Type");
              break;
              
            case 3:
              //Decoder found
              lcd.print(DecoderTyp);
              break;
  
         }
         break;
         
      }
      
    } else {
      // not ESU
      
      if (CVNr != 0) {
        if (CVNr == CVType) {
          lcd.setCursor(2, 2);
          if (strcmp(ExtFile, "") == 0) {
            //kein File angegeben
            lcd.print(CVVal);
            lcd.print(" - no Data");
          } else {
            int retVal = GetDecoderTyp(CVVal, false);
            switch(retVal) {
              case 0:
                //SD Card not found
                lcd.print(CVVal);
                lcd.print(" - no Card");
                break;
                
              case 1:
                //CSV File not Found
                lcd.print(CVVal);
                lcd.print(" - no File");
                break;
                
              case 2:
                //Decoder not found
                lcd.print(CVVal);
                lcd.print(" - no Type");
                break;
                
              case 3:
                //Decoder found
                lcd.print(DecoderTyp);
                break;
    
            }
          }
          
          if (CVVersion != 0) {
            NextNr = CVVersion;
          }
          lcd.setCursor(13, 0);
        } else if (CVNr == CVVersion) {
          ValVersion = CVVal;
          if (CVSubversion != 0) {
            NextNr = CVSubversion;
          }
        } else if (CVNr == CVSubversion) {
          ValSubversion = CVVal;
          if (CVBuild != 0) {
            NextNr = CVBuild;
          }
        } else if (CVNr == CVBuild) {
          ValBuild = CVVal;
        }
      }
      
      switch(CVNr) {
        case 0:
          //Start Reading
          // ToDo: Set Display
          UpdateProgLCD(true); // Display zurücksetzen
          // Variablen zurücksetzen
          DecoderValue = 0;
          Manufacturer[0] = '\0';
          DecoderTyp[0] = '\0';
          CVType = 0;
          CVVersion = 0;
          CVSubversion = 0;
          CVBuild = 0;
          ValVersion = 0;
          ValSubversion = 0;
          ValBuild = 0;
          ExtFile[0] = '\0';
          
          NextNr = 8; //Hersteller
          break;
  
        case 8:
          // Hersteller gelesen
          lcd.setCursor(2, 1);
          NextNr = 0;
  
          if (CVVal == 151) {
            // ESU nicht aus der CSV lesen, die Daten aufzubereiten ist echt schräg
            DecoderValue = CVVal;
            lcd.print("ESU");
            NextNr = 31; //Decoder Register
            CVVal = 0;
            flgWrite = true;
            break;
          }
          
          int retVal = GetManufacturer(CVVal);
          switch(retVal) {
            case 0:
              //SD Card not found
              lcd.print("Card not found");
              break;
              
            case 1:
              //CSV File not Found
              lcd.print("File not found");
              break;
              
            case 2:
              //Decoder not found
              lcd.print("Decoder not found");
              break;
              
            case 3:
              //Decoder found
              lcd.print(Manufacturer);
              if (CVType == 0) {
                lcd.setCursor(2, 2);
                lcd.print("unknown");
                lcd.setCursor(13, 0);
              } else {
                NextNr = CVType; //Typ
              }
              if (NextNr == 0) {
                if (CVVersion != 0) {
                  NextNr = CVVersion;
                }
              }
              break;
              
          }
          lcd.setCursor(13, 0);
          break;
          
      }

    }
    
    if (NextNr > 0) {
      int tmp_CB_VAR = CB_NUM_GETINFO;
      if (!flgWrite) {
        updateCVMsg("Read");
        mySerial.print("<R ");
      } else {
        updateCVMsg("Write");
        mySerial.print("<W ");
      }
      mySerial.print(NextNr);
      mySerial.print(" ");
      if (flgWrite) {
        mySerial.print(CVVal);
        mySerial.print(" ");
      }
      mySerial.print(DEVICE_ID);
      mySerial.print(" ");
      if (!flgWrite) bitWrite(tmp_CB_VAR, CB_READ_BYTE, 1);
      else bitWrite(tmp_CB_VAR, CB_WRITE_BYTE, 1);
      mySerial.print(tmp_CB_VAR);
      mySerial.print(">");
      lastCV = millis();
      lastDatas = DataTimeout + millis(); // dont get other datas
      CVSend = 1;
      return false;
    } else {
      // ToDo: 
      lcd.setCursor(2, 3);
      if (ValVersion == 0 && ValSubversion == 0 && ValBuild == 0) {
        lcd.print("no Data");
      } else {
        lcd.print(ValVersion);
        lcd.print(".");
        lcd.print(ValSubversion);
        lcd.print(".");
        lcd.print(ValBuild);
      }
      lcd.setCursor(13, 0);
      updateCVMsg("R:Ok");
      CVSend++;
      return true;
    }
    
}
#endif
