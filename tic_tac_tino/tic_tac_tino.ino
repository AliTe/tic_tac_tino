/* TIC TAC TINO */

#include "tictactino.h"

int latchPin = 3;
int clockPin = 4;
int dataPin = 5;
int greenPin = 7;
int redPin = 8;

uint32_t matrix;
void demo(uint32_t &data)
{
  if (data > 0) data >>= 1;
  else data = 131072;
  delay(100);
}

void setup()
{
  matrix = 262144;
  tictactino::init(greenPin, redPin, dataPin, clockPin, latchPin);
  tictactino::demo(demo, matrix);
}

void loop()
{
  tictactino::play();
  tictactino::show();
}
