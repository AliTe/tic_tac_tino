#ifndef TICTACTINO_H
#define TICTACTINO_H

#include "Arduino.h"

#define _DEBUG 1

namespace tictactino {
  void init(int dataPin, int clockPin, int latchPin, int greenPin, int redPin);
  void play();
  void show();
  //void reset();
  //Status status();
};

/*
class Tictactino
{
  public:
    
    enum Player { GREEN, RED, BLINK };
    
  private:
    enum Turn { NONE, MOVE, SET };
    // Members
    uint16_t _field[3];
    uint16_t _winpattern;
    static const uint16_t _wmasks[8];
    static const int _inpins[2];
    uint32_t _register;
    uint8_t _counter;
    State _state;
    unsigned long _blinkalarm;
    unsigned long _inputalarm;
    uint16_t _cursor;
    uint16_t _lastInput;
    
    // Methods
    void _refresh();
    void _turn(Player p);
    void _move(Player p);
    void _check();
    Player _getCurrentPlayer();
    Turn _getInput(Player p);
    
  public:
    // Methods
    Tictactino();
    void init();
    void play();
    void show();
};
*/
#endif
