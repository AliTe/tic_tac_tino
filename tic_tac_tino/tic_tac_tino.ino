/* TIC TAC TINO */

#include "tictactino.h"

int latchPin = 3;
int clockPin = 4;
int dataPin = 5;
int greenPin = 7;
int redPin = 8;

void setup()
{
  tictactino::init(greenPin, redPin, dataPin, clockPin, latchPin);
}

void loop()
{
  tictactino::play();
  tictactino::show();
}
