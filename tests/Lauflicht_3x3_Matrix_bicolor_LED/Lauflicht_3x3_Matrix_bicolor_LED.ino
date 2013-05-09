/*
Testaufbau: 3x3 Matrix bicolor LEDs
Lauflicht gruen / rot
angesteuert ueber 3 x 8bit Shif Register
*/
 
int latchPin = 3;
int clockPin = 4;
int dataPin = 5;
 
uint32_t leds;
 
void setup() 
{
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
}
 
void loop() 
{
  leds = 1;
  //delay(100);
  for (int i = 0; i < 18; i++)
  {
    Serial.println(leds);
    updateShiftRegister();
    leds <<= 1;
    delay(75);
  }
}
 
void updateShiftRegister()
{
   byte b = 0;
   digitalWrite(latchPin, LOW);
   for (int i = 2; i >= 0; --i) {
     b = (byte) ((leds >> (i * 8)) & 255);
     shiftOut(dataPin, clockPin, MSBFIRST, b);
   }
   digitalWrite(latchPin, HIGH);
}

