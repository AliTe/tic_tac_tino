#ifndef TICTACTINO_H
#define TICTACTINO_H

#include "Arduino.h"

namespace tictactino {
  void init(int greenPin, int redPin, int dataPin, int clockPin, int latchPin);
  void play(); // enter the game loop
  void show(); // show game result, waiting for keypress to start new game
};

#endif
