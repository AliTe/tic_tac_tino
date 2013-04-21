/* ttttest */

#include "tictactino.h"

Tictactino game;

void setup()
{
  #ifdef _DEBUG
  Serial.begin(9600);
  #endif
  pinMode(2, INPUT);
  pinMode(3, INPUT);
}

void loop()
{
  game.init();  
  game.play();
  game.show();
}
