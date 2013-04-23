#include "tictactino.h"

namespace tictactino {
  enum Status { UNDEFINED, RESET, RUNNING, WIN_GREEN, WIN_RED, EQUAL, DEMO };
  enum Player { GREEN, RED };
  enum Input { NONE, MOVE, SET };
  const uint16_t winmask[8] = { 7, 56, 73, 84, 146, 273, 292, 448 };
  volatile Status state = UNDEFINED;
  
  uint16_t playground[2] = { 0, 0 };
  int dataP, clockP, latchP, inputP[2];
  uint32_t playgroundRegister = 0;
  uint8_t counter = 0;
  unsigned long inputtime, blinktime;
  uint16_t cursor, lastinput, blinkmask, winpattern;
  
  Player getPlayer();
  void reset();
  void refresh();
  Input getInput(Player p);
  void move(Player p);
  void check();
  void turn(Player p);
  
  void init(int dataPin, int clockPin, int latchPin, int greenPin, int redPin)
  {
    #ifdef _DEBUG
    Serial.begin(9600);
    Serial.println(); Serial.println(); Serial.println(); Serial.println();
    Serial.println(); Serial.println("===============");
    Serial.println("TIC TAC TINO"); Serial.println("---------------");
    Serial.println("Starte Initialisierung"); Serial.println();
    #endif
    dataP = dataPin;
    clockP = clockPin;
    latchP = latchPin;
    inputP[GREEN] = greenPin;
    inputP[RED] = redPin;
    pinMode(inputP[GREEN], INPUT);
    pinMode(inputP[RED], INPUT);
    pinMode(dataP, OUTPUT);
    pinMode(clockP, OUTPUT);
    pinMode(latchP, OUTPUT);
    #ifdef _DEBUG
    Serial.println("Initialisierung abgeschlossen"); Serial.println("---------------"); Serial.println();
    #endif
  }
  
  void reset()
  {
    #ifdef _DEBUG
    Serial.println("Bereite Spielfeld vor"); Serial.println("---------------"); Serial.println();
    #endif
    state = UNDEFINED;
    playgroundRegister = 0;
    counter = 0;
    blinktime =inputtime = 0;
    blinkmask = cursor = 1;
    lastinput = LOW;
    winpattern = 0;
    playground[GREEN] = playground[RED] = 0;
    state = RESET;
    #ifdef _DEBUG
    Serial.println("Spielfeld ist bereit fuer neues Spiel"); Serial.println("---------------"); Serial.println();
    #endif
  }
  
  void play()
  {
    reset();
    #ifdef _DEBUG
    Serial.println("Beginne neues Spiel:"); Serial.println("---------------"); Serial.println();
    #endif
    state = RUNNING;
    refresh();
    while (state == RUNNING) {
      turn(getPlayer());
    }
  }
  
  Player getPlayer()
  {
    if ((counter & 1) == 1) return RED;
    else return GREEN;
  }
  
  void turn(Player p)
  {
    switch (getInput(p)) {
      case MOVE:
        move(p);
        break;
      case SET:
        blinkmask = 0;
        playground[p] |= cursor;
        check();
        if (state == RUNNING) {
          if (p == GREEN) cursor = 1;
          else cursor = 256;
          counter++;
          move(getPlayer());
        }
        break;
    }
  }
  
  Input getInput(Player p)
  {
    Input result = NONE;
    int input = digitalRead(inputP[p]);
    if (lastinput == LOW) {
      // new keypress detected
      if (input == HIGH) {
        // set inputtime - to check if key was pressed long or short
        inputtime = millis() + 1200;
        lastinput = HIGH;
      }
    }
    else {
      // key was pressed before
      if (input == LOW && inputtime >= millis()) {
        // keypress was short -> move corsor
        inputtime = 0;
        lastinput = LOW;
        result = MOVE;
      }
      else if (input == LOW && inputtime < millis()) {
        // keypress was long -> set
        inputtime = 0;
        lastinput = LOW;
        result = SET;
      }
    }
    delay(50); // entprellen
    return result;
  }
  
  void move(Player p)
  {
    uint16_t mask = (playground[GREEN] | playground[RED]);
    if (p == GREEN) {
      do {
        cursor = cursor << 1;
        if (cursor > 256) cursor = 1;
      } while ((cursor & mask) > 0);
    }
    else {
      do {
        cursor = cursor >> 1;
        if (cursor <= 0) cursor = 256;
      } while ((cursor & mask) > 0);
    }
    blinkmask = cursor;
    refresh();
  }
  
  void check()
  {
    Serial.println(cursor); Serial.println(getPlayer());
    // playground full? Set last remaining field
    if (counter == 8) {
      playground[getPlayer()] |= cursor;
    }
    for (int i = 0; i < 8; ++i) {
      if((playground[GREEN] & winmask[i]) == winmask[i]) {
        winpattern = winmask[i];
        state = WIN_GREEN;
      }
      else if ((playground[RED] & winmask[i]) == winmask[i]) {
        winpattern = winmask[i];
        state = WIN_RED;
      }
    }
    if (counter >= 8 && winpattern == 0) state = EQUAL;
  }
  
  void refresh()
  {
    #ifdef _DEBUG
    Serial.print("Spielzug: "); Serial.print(counter); Serial.print(" - ");
    if (getPlayer() == GREEN) Serial.println("GRUEN");
    else Serial.println("ROT");
    Serial.println("----------");
    for  (int i = 0; i < 9 ; i++) {
      if (i % 3 == 0) {
        Serial.println();
        Serial.print("   ");
      }
      if (bitRead(blinkmask, i)) Serial.print("*");
      else if (bitRead(playground[GREEN], i)) Serial.print("G");
      else if (bitRead(playground[RED], i)) Serial.print("R");
      else Serial.print(".");
    }
    Serial.println(); Serial.println(); Serial.println("----------");
    #endif
  }
  
  void show()
  {
    blinkmask = winpattern;
    #ifdef _DEBUG
    if (state == WIN_RED) {
      Serial.println("Gewinner: ROT");
    }
    else if (state == WIN_GREEN) {
      Serial.println("Gewinner ist: GRUEN");
    }
    else Serial.println("Unentschieden !!!");
    refresh();
    Serial.println("Beliebigen Taster druecken, um Spiel neu zu starten ...");
    #endif
    do {
      if (lastinput == LOW && (digitalRead(inputP[GREEN]) == HIGH ||
          digitalRead(inputP[RED]) == HIGH)) lastinput = HIGH;
      else if (lastinput == HIGH && digitalRead(inputP[GREEN]) == LOW && digitalRead(inputP[RED]) == LOW) {
        #ifdef _DEBUG
        Serial.println("Fuehre RESET durch");
        Serial.println("---------------"); Serial.println();
        #endif
        lastinput = LOW;
        state = RESET;
      }
      delay(50);
    } while (state != RESET);
  }
}

/*

const int Tictactino::_inpins[2] = { 2, 3 };

Tictactino::Tictactino() {
}

void Tictactino::init() {
  #ifdef _DEBUG
  Serial.println(); Serial.println(); Serial.println(); Serial.println();
  Serial.println(); Serial.println("===============");
  Serial.println("NEUES SPIEL"); Serial.println("---------------");
  Serial.println("Starte Initialisierung"); Serial.println();
  #endif
  _state = UNDEFINED;
  _register = 0;
  _counter = 0;
  _blinkalarm = 0;
  _inputalarm = 0;
  _lastInput = 0;
  _cursor = 1;
  _winpattern = 0;
  _field[GREEN] = 0;
  _field[RED] = 0;
  _field[BLINK] = _cursor;
  _state = STARTING;
  #ifdef _DEBUG
  Serial.println("Initialisierung abgeschlossen"); Serial.println("---------------"); Serial.println();
  #endif
}


void Tictactino::play() {
  _state = RUNNING;
  #ifdef _DEBUG
  Serial.println("Spielstart"); Serial.println("---------------"); Serial.println();
  #endif
  _refresh();
  while (_state == RUNNING) {
    // entering game main loop
    _turn(_getCurrentPlayer());
  }
  #ifdef _DEBUG
  Serial.println("Spielende"); Serial.println("---------------"); Serial.println();
  #endif
}

void Tictactino::_check() {
  if (_counter == 8) {
    _field[_getCurrentPlayer()] |= _cursor;
  }
  for (int i = 0; i < 8; ++i) {
    if ((_field[GREEN] & _wmasks[i]) == _wmasks[i]) {
      _winpattern = _wmasks[i];
      #ifdef _DEBUG
      Serial.print("Green: "); Serial.print(_field[GREEN], BIN);
      Serial.print(" Red: "); Serial.print(_field[RED], BIN);
      Serial.print(" Mask: "); Serial.print(_wmasks[i], BIN);
      Serial.print(" Cursor: "); Serial.println(_cursor, BIN);
      Serial.println("---------------"); Serial.println();
      #endif
      _state = WIN_GREEN;
      break;
    }
    else if ((_field[RED] & _wmasks[i]) == _wmasks[i]) {
      _winpattern = _wmasks[i];
      #ifdef _DEBUG
      Serial.print("Green: "); Serial.print(_field[GREEN], BIN);
      Serial.print(" Red: "); Serial.print(_field[RED], BIN);
      Serial.print(" Mask: "); Serial.print(_wmasks[i], BIN);
      Serial.print(" Cursor: "); Serial.println(_cursor, BIN);
      Serial.println("---------------"); Serial.println();
      #endif
      _state = WIN_RED;
      break;
    }
  }
  if (_winpattern == 0 && _counter >= 8) _state = EQUAL;
}

void Tictactino::show() {
  _field[BLINK] = _winpattern;
  #ifdef _DEBUG
  if (_state == WIN_RED) {
    Serial.println("Gewinner: ROT");
  }
  else if (_state == WIN_GREEN) {
    Serial.println("Gewinner ist: GRUEN");
  }
  else Serial.println("Unentschieden !!!");
  _refresh();
  Serial.println("Beliebigen Taster druecken, um Spiel neu zu starten ...");
  #endif
  do {
    if (_lastInput == LOW && (digitalRead(_inpins[GREEN]) == HIGH || digitalRead(_inpins[RED]) == HIGH)) {
      _lastInput = HIGH;
    }
    else if (_lastInput == HIGH && digitalRead(_inpins[GREEN]) == LOW && digitalRead(_inpins[RED]) == LOW) {
      #ifdef _DEBUG
     Serial.println("Fuehre RESET durch");
     Serial.println("---------------"); Serial.println();
     #endif
     _lastInput = LOW;
     _state = RESET;
    }
    delay(50);
  } while (_state != RESET);
}


void Tictactino::_refresh() {
  #ifdef _DEBUG
  Serial.println("----------"); Serial.print("Spielzug: "); Serial.print(_counter); Serial.print(" - ");
  if (_getCurrentPlayer() == GREEN) Serial.println("GRUEN");
  else Serial.println("ROT");
  Serial.println("----------");
  for  (int i = 0; i < 9 ; i++) {
    if (i % 3 == 0) {
      Serial.println();
      Serial.print("   ");
    }
    if (bitRead(_field[BLINK], i)) Serial.print("*");
    else if (bitRead(_field[GREEN], i)) Serial.print("G");
    else if (bitRead(_field[RED], i)) Serial.print("R");
    else Serial.print(".");
  }
  Serial.println(); Serial.println(); Serial.println("----------");
  #endif
}

Tictactino::Player Tictactino::_getCurrentPlayer() {
  if (_counter % 2 == 1) return RED;
  else return GREEN;
}

void Tictactino::_turn(Player p) {
  switch (_getInput(p)) {
    case MOVE:
      #ifdef _DEBUG
      Serial.print("Spieler ");
      if (p == GREEN) Serial.print("GRUEN");
      else Serial.print("ROT");
      Serial.print(" ist auf Position "); Serial.print(_cursor, BIN); Serial.println(" gesprungen.");
      Serial.println("----------"); Serial.println();
      #endif
      _move(p);
      //
      //_refresh();
      break;
    
    case SET:
      // set
      #ifdef _DEBUG
      Serial.print("Spieler ");
      if (p == GREEN) Serial.print("GRUEN");
      else Serial.print("ROT");
      Serial.print(" hat Position "); Serial.print(_cursor, BIN); Serial.println(" gesetzt.");
      Serial.println("----------"); Serial.println();
      #endif
      _field[BLINK] = 0;
      _field[p] |= _cursor;
      _check();
      if (_state == RUNNING) {
        if (p == GREEN) _cursor = 1;
        else _cursor = 256;
        _counter++;
        _move(_getCurrentPlayer());
      }
      break;
  }
}


Tictactino::Turn Tictactino::_getInput(Player p) {
  Turn _ret = NONE;
  int _input = digitalRead(_inpins[p]);
  if (_lastInput == LOW) {
    // nothing changed
    if (_input == LOW) {
      //_inputalarm = 0;
      //_ret = NONE;
    }
    // new keypress detected
    else {
      // set alarm (to check if key press was short or long
      _inputalarm = millis() + 1500;
      _lastInput = HIGH;
      //_ret = NONE;
    }
  }
  else {
    // key was pressed before
    if (_input == LOW && _inputalarm >= millis()) {
      // key released after short press -> move cursor
      _inputalarm = 0;
      _lastInput = LOW;
      //_move(p);
      _ret = MOVE;
    }
    else if (_input == LOW && _inputalarm <= millis()) {
      // key released after long press -> set
      _inputalarm = 0;
      _lastInput = LOW;
      _ret = SET;
    }
  }
  delay(50);
  return _ret;
}

void Tictactino::_move(Player p) {
  uint16_t _mask = (_field[GREEN] | _field[RED]);
  if (p == GREEN) {
    do {
      _cursor = _cursor << 1;
      if (_cursor > 256) _cursor = 1;
    } while ((_cursor & _mask) > 0);
  }
  else {
    do {
      _cursor = _cursor >> 1;
      if (_cursor <= 0) _cursor = 256;
    } while ((_cursor & _mask) > 0);
  }
  //
  _field[BLINK] = _cursor;
  _refresh();
}
*/
