#if (defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560))

int GetManufacturer(int DecoderNumber) {
  if (isSDCARD) {
    bool flgFound = false;
    myFile = SD.open("decoder.csv");
    if (myFile) {
      while (myFile.available()) {
        char tmp_buf[19];
        String buffer = myFile.readStringUntil(';');
        buffer.toCharArray(tmp_buf, sizeof(tmp_buf));
        DecoderValue = atoi(tmp_buf);
        if (DecoderValue == DecoderNumber) {
          buffer = myFile.readStringUntil(';');
          buffer.toCharArray(Manufacturer, sizeof(Manufacturer));
          buffer = myFile.readStringUntil(';');
          buffer.toCharArray(tmp_buf, sizeof(tmp_buf));
          CVType = atoi(tmp_buf);
          buffer = myFile.readStringUntil(';');
          buffer.toCharArray(tmp_buf, sizeof(tmp_buf));
          CVVersion = atoi(tmp_buf);
          buffer = myFile.readStringUntil(';');
          buffer.toCharArray(tmp_buf, sizeof(tmp_buf));
          CVSubversion = atoi(tmp_buf);
          buffer = myFile.readStringUntil(';');
          buffer.toCharArray(tmp_buf, sizeof(tmp_buf));
          CVBuild = atoi(tmp_buf);
          buffer = myFile.readStringUntil('\n');
          buffer.trim();
          buffer.toCharArray(ExtFile, sizeof(ExtFile));
          flgFound = true;
          break;
        } else {
          myFile.readStringUntil('\n');
        }
      }
      // close the file:
      myFile.close();
      if (flgFound) return 3; //Decoder found
      else return 2;          //Decoder not found
    } else {
      // if the file didn't open, print an error:
      return 1;               // CSV file not found
    }
  } else {
    return 0;                 // SD-Card not initialized
  }
}

int GetDecoderTyp(long Typ, bool isHex) {
  // isHex -> bei ESU ist die Liste als HEX Zahl gespeichert ...
  if (isSDCARD) {
    bool flgFound = false;
    myFile = SD.open(ExtFile);
    if (myFile) {
      while (myFile.available()) {
        char tmp_buf[19];
        String buffer = myFile.readStringUntil(';');
        if (!isHex) {
          buffer.toCharArray(tmp_buf, sizeof(tmp_buf));
          if (atoi(tmp_buf) == Typ) {
            buffer = myFile.readStringUntil('\n');
            buffer.trim();
            buffer.toCharArray(DecoderTyp, sizeof(DecoderTyp));
            flgFound = true;
            break;
          } else {
            myFile.readStringUntil('\n');
          }
        } else {
          if (hexToDec(buffer) == Typ) {
            buffer = myFile.readStringUntil('\n');
            buffer.trim();
            buffer.toCharArray(DecoderTyp, sizeof(DecoderTyp));
            flgFound = true;
            break;
          } else {
            myFile.readStringUntil('\n');
          }
        }
      }
      // close the file:
      myFile.close();
      if (flgFound) return 3; //Type found
      else return 2;          //Type not found
    } else {
      // if the file didn't open, print an error:
      return 1;               // CSV file not found
    }
  } else {
    return 0;                 // SD-Card not initialized
  }
}

//https://github.com/benrugg/Arduino-Hex-Decimal-Conversion/blob/master/hex_dec.ino
long hexToDec(String hexString) {
  
  long decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}
#endif
