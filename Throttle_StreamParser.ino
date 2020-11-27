
void StreamParser_flush() {
    bufferLength=0;
    inCommandPayload=false;   
}

void StreamParser_loop(Stream & stream) {
   while(stream.available()) {
    if (bufferLength==MAX_BUFFER) {
       StreamParser_flush();
    }
    char ch = stream.read();
    if (ch == '<') {
      inCommandPayload = true;
      bufferLength=0;
      buffer[0]='\0';
    } 
    else if (ch == '>') {
      buffer[bufferLength]='\0';
      StreamParser_parse( & stream, buffer, false); // Parse this allowing async responses
      inCommandPayload = false;
      break;
    } else if(inCommandPayload) {
      buffer[bufferLength++]= ch;
    }
  }
}

int StreamParser_splitValues( int result[MAX_PARAMS], const byte * cmd) {
  byte state=1;
  byte parameterCount=0;
  int runningValue=0;
  const byte * remainingCmd=cmd+1;  // skips the opcode
  bool signNegative=false;
  
  // clear all parameters in case not enough found
  for (int i=0;i<MAX_PARAMS;i++) result[i]=0;
  
  while(parameterCount<MAX_PARAMS) {
      byte hot=*remainingCmd;
      
       switch (state) {
    
            case 1: // skipping spaces before a param
               if (hot==' ' || hot=='|') break;
               if (hot == '\0' || hot=='>') return parameterCount;
               state=2;
               continue;
               
            case 2: // checking sign
               signNegative=false;
               runningValue=0;
               state=3; 
               if (hot!='-') continue; 
               signNegative=true;
               break; 
            case 3: // building a parameter   
               if (hot>='0' && hot<='9') {
                   runningValue=10*runningValue+(hot-'0');
                   break;
               }
               if (hot>='A' && hot<='Z') {
                   // Since JMRI got modified to send keywords in some rare cases, we need this
                   // Super Kluge to turn keywords into a hash value that can be recognised later
                   runningValue = ((runningValue << 5) + runningValue) ^ hot;
                   break;
               }
               result[parameterCount] = runningValue * (signNegative ?-1:1);
               parameterCount++;
               state=1; 
               continue;
         }   
         remainingCmd++;
      }
      return parameterCount;
}

// See documentation on DCC class for info on this section
void StreamParser_parse(Print * stream,  byte *com, bool blocking) {
    int p[MAX_PARAMS];
    while (com[0]=='<' || com[0]==' ') com++; // strip off any number of < or spaces
    byte params=StreamParser_splitValues(p, com); 
    byte opcode=com[0];
    bool doRefresh = false;
    
    // Functions return from this switch if complete, break from switch implies error <X> to send
    switch(opcode) {
    case '\0': return;    // filterCallback asked us to ignore 
    case 't':       // THROTTLE <t CAB SPEED DIRECTION FUNCTION_LOW FUNCTION_HIGH>
        if (params == 5) {
          int cab = p[0];
          int speed = p[1];
          int dir = p[2];
          int i;
          unsigned long func = (p[3] & 0xFFFF) + ((unsigned long)p[4] << 16);
          bool found = false;
          for (i = 0; i < maxLocos; i++) {
            if (LocoAddress[i] == cab) {
              found = true;
              break;
            }
          }
          if (found) {
            if (LocoDirection[i] != dir) {
              LocoDirection[i] = dir;
              LocoZeroCount[i] = reverse_direction;
              doRefresh = true;
            }
            if (LocoSpeed[i] != speed) {
              LocoSpeed[i] = speed;
              doRefresh = true;
            }
            if (doRefresh && !CVPROG) updateSpeedsLCD(i);

            //Masked Out LocoPush
            unsigned long mask_func = (~LocoPushFunction[i]) & func;
            
            if (LocoFunction[i] != mask_func) {
              LocoFunction[i] = mask_func;
              if ((ActiveAddress == i) && !CVPROG) InitialiseFunctionLCD();
            }
            re_absolute = LocoSpeed[ActiveAddress];
            cmd_count--;
          }
          return;
          
        }
        break;

     case 'p':
        if (params == 2) {
          switch (p[1]) {
           case HASH_KEYWORD_MAIN:
              cmd_count--;
              if (track_power != (bool) p[0]) {
                track_power = ! track_power;
                if (!CVPROG) UpdateFunctionLCD(29);
              }
              return;
          
          case HASH_KEYWORD_PROG:
              return;
              
          case HASH_KEYWORD_JOIN:
              return;
          
          }
          break;
        }
        break;

      case 'r':   // return from CV Prog
        if (params == 4) {
          if (p[0] != DEVICE_ID) break;
          int tmp_CB_VAR = p[1];
          bitWrite(tmp_CB_VAR, CB_READ_BYTE, 0);
          bitWrite(tmp_CB_VAR, CB_WRITE_BYTE, 0);
          if (bitRead(p[1], CB_READ_BYTE) == 1) {
              // < r CALLBACKNUM|CALLBACKSUB|CV VALUE>
              if (p[3] == -1) {
                updateCVMsg("R:Err");
              } else {
                updateCVMsg("R:Ok");

                String(CVVal).toCharArray(CVValArr, sizeof(CVValArr));
                if (CVPROG) {
                  if (tmp_CB_VAR == CB_NUM_GETADDR) {
                    //read decoder adress
                    if (GetDecoderAddr(p[2],p[3])) return;
                  } else if (tmp_CB_VAR == CB_NUM_SETADDR) {
                    //read decoder adress
                    if (SetDecoderAddr(p[2] * 10,p[3])) return;
                  } else if (tmp_CB_VAR == CB_NUM_GETINFO) {
                    //read decoder info
                    #if (defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560))
                      if (GetDecoderInfo(p[2],p[3])) return;
                    #endif
                  } else {
                    CVNumber = p[2];
                    CVVal = p[3];
                    updateCVLCD(CVVal, 2);
                    updateCVHexLCD();
                    updateCVBinLCD();
                    lcd.setCursor(CVCol, CVLine);
                  }
                }
              }
              CVSend++;
              lastCV = millis();
              lastDatas = DataTimeout + millis(); // dont get other datas
              return;
           } else if (bitRead(p[1], CB_WRITE_BYTE) == 1) {
              // < r CALLBACKNUM|CALLBACKSUB|CV VALUE>
              if (!CVPROG) break;
              if (p[3] == -1) {
                updateCVMsg("W:Err");
              } else {
                if (tmp_CB_VAR == CB_NUM_SETADDR) {
                  //read decoder adress
                  // if return == true, Address was read
                  if (SetDecoderAddr(p[2],p[3])) return;                
                } else if (tmp_CB_VAR == CB_NUM_GETINFO) {
                  //read decoder info -> ESU must write
                  #if (defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560))
                    if (GetDecoderInfo(p[2],p[3])) return;
                  #endif
                } else {
                  if (CVVal == p[3]) {
                    updateCVMsg("W:Ok");
                  } else {
                    updateCVMsg("W:Verr");  // Verify Error
                  }
                }
                lcd.setCursor(CVCol, CVLine);
              }
              CVSend++;
              lastCV = millis();
              lastDatas = DataTimeout + millis(); // dont get other datas
              return;
           }
        }
        break;
        
   }
}

void StreamParser_PrintFunction(unsigned long func) {
  Serial.print("Functions: ");
  for (int i = 0; i < 29; i++) {
    byte flg = bitRead(func, i);
    Serial.print(flg);
  }
  Serial.println("");
}
