#include "tictactino.h"

const uint16_t Tictactino::_wmasks[8] = { 7, 56, 73, 84, 146, 273, 292, 448 };
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
    /*
    else {
      // key is still pressed
      _ret = NONE;
    }
    */
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

//void Tictactino::
