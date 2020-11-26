
#define CLK     A0
#define DATA    A1
#define BUTTON  A2

void setup() {
  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT_PULLUP);
  pinMode(BUTTON, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  Serial.begin (115200);
  Serial.println("Rotary Test:");
}

static uint8_t prevNextCode = 0;

void loop() {
uint32_t pwas=0;

   if( read_rotary() ) {

      if ( (prevNextCode&0x0f)==0x0b) Serial.println("Forward");
      if ( (prevNextCode&0x0f)==0x07) Serial.println("Backward");
   }

   if (digitalRead(BUTTON)==0) {

      delay(10);
      if (digitalRead(BUTTON)==0) {
          Serial.println("Push Button");
          while(digitalRead(BUTTON)==0);
      }
   }
}

// A vald CW or CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prevNextCode <<= 2;
  if (digitalRead(DATA)) prevNextCode |= 0x02;
  if (digitalRead(CLK)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

  return ( rot_enc_table[( prevNextCode & 0x0f )]);
}
