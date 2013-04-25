#include "tictactino.h"

namespace tictactino {
  enum Player { GREEN, RED };
  enum Status { UNDEF, RESET, RUNNING, WIN_GREEN, WIN_RED, EQUAL, DEMO };
  enum Command { NOOP, MOVE, SET };
  const uint16_t winmask[8] = { 7, 56, 73, 84, 146, 273, 292, 448 };
  int inputPins[2], dPin, cPin, lPin, lastinput;
  uint8_t counter = 0;
  uint16_t playground[2], blinkmask, cursor, winpattern;
  uint32_t _register;
  unsigned long inputtimer, blinktimer;
  volatile Status status = UNDEF;
  void reset();
  void refresh();
  void turn(Player p);
  Command read(Player p);
  void move(Player p);
  void set(Player p);
  Player player();
  
  void init(int greenPin, int redPin, int dataPin, int clockPin, int latchPin)
  {
    inputPins[GREEN] = greenPin; inputPins[RED] = redPin;
    dPin = dataPin; cPin = clockPin; lPin = latchPin;
    #ifdef _DEBUG
    Serial.begin(9600);
    Serial.println("================"); Serial.println("| TIC TAC TINO |");
    Serial.println("================"); Serial.println();
    #endif
  }
  
  void reset()
  {
    blinkmask = cursor = 1;
    winpattern = 0;
    lastinput = LOW;
    playground[GREEN] = playground[RED] = 0;
    inputtimer = blinktimer = 0;
    counter = 0;
    _register = 0;
    #ifdef _DEBUG
    Serial.println(); Serial.println("-----------"); Serial.println("Neues Spiel");
    Serial.println("-----------"); Serial.println();
    #endif
    status = RESET;
  }
    
  Player player()
  {
    if ((counter & 1) == 0) return GREEN;
    else return RED;
  }

  void play()
  {
    reset();
    refresh();
    status = RUNNING;
    while (status == RUNNING) {
      // enter game main loop
      turn(player());
    }
  }
  
  void turn(Player p)
  {
    switch (read(p)) {
      case MOVE:
        move(p);
        break;
        
      case SET:
        set(p);
        break;
        
      case NOOP:
        break;
    }
  }
  
  Command read(Player p)
  {
    Command cmd = NOOP;
    int input = digitalRead(inputPins[p]);
    if (lastinput == LOW) {
      // key press detected
      if (input == HIGH) {
        lastinput = HIGH;
        inputtimer = millis() + 1000;
      }
      // else: nothing happened
    } // key was pressed
    else if (input == LOW && inputtimer <= millis()) {
      // and released after long time
      cmd = SET;
      lastinput = LOW;
    }
    else if (input == LOW && inputtimer > millis()) {
      // or released after short time
      cmd = MOVE;
      lastinput = LOW;
    }
    delay(50); // entprellen
    return cmd;
  }
  
  void move(Player p)
  {
    uint16_t mask = (playground[GREEN] | playground[RED]);
    //int recounter = 0;
    switch (p) {
      case GREEN:
        do {
          cursor <<= 1;
          if (cursor > 256) cursor = 1;
        } while ((mask & cursor) != 0);
        break;
        
      case RED:
        do {
          cursor >>= 1;
          if (cursor <= 0) cursor = 256;
        } while ((mask & cursor) != 0);
        break;
    }
    blinkmask = cursor;
    refresh();
  }
  
  void refresh()
  {
    #ifdef _DEBUG
    for (int i = 0; i < 9; ++i) {
      if (i % 3 == 0) {
        Serial.println();
        Serial.print("   ");
      }
      if (bitRead(blinkmask, i)) Serial.print(" * ");
      else if (bitRead(playground[GREEN], i)) Serial.print(" X ");
      else if (bitRead(playground[RED], i)) Serial.print(" O ");
      else Serial.print(" . ");
    }
    Serial.println(); Serial.println();
    #endif
  }
  
  void set(Player p)
  {
    playground[p] |= cursor;
    blinkmask = 0;
    counter++;
    p = player();
    if (counter <= 8) {
      if (p == GREEN) cursor = 256;
      else cursor = 1;
      move(p);
      #ifdef _DEBUG
      Serial.print("Naechster Zug: ");
      switch (p) {
        case GREEN:
          Serial.println("Gruen");
          break;
        case RED:
          Serial.println("Rot");
          break;
      }
      #endif
    }
    // check if someone is winner
    for (int i = 0; i < 8; ++i) {
      if ((playground[GREEN] & winmask[i]) == winmask[i]) {
        winpattern = winmask[i];
        status = WIN_GREEN;
        break;
      }
      else if ((playground[RED] & winmask[i]) == winmask[i]) {
        winpattern = winmask[i];
        status = WIN_RED;
        break;
      }
    }
    if (winpattern == 0 && counter >= 8) {
      if (counter == 8) {
        playground[p] |= cursor;
        set(p);
        blinkmask = cursor = 0;
        return;
      }
      else status = EQUAL;
    }
    refresh();
  }
  
  
  void show()
  {
    #ifdef _DEBUG
    Serial.println(); Serial.println("-----------"); Serial.println("Spiel zu Ende");
    Serial.println("-----------"); Serial.println();
    #endif
    cursor = 0;
    blinkmask = winpattern;
    refresh();
    #ifdef _DEBUG
    switch (status) {
      case WIN_RED:
        Serial.print("Rot hat gewonnen ");
        break;
      case WIN_GREEN:
        Serial.print("Gruen hat gewonnen ");
        break;
      case EQUAL:
        Serial.print("Unentschieden ");
        break;
    }
    Serial.print("nach dem ");
    Serial.print(counter);
    Serial.println(" Spielzug");
    Serial.println("Beliebe Taste druecken, um neues Spiel zu starten ...");
    #endif
    do {
        if (lastinput == LOW && (digitalRead(inputPins[GREEN]) == HIGH || digitalRead(inputPins[RED]) == HIGH))
          lastinput = HIGH;
        else if (lastinput == HIGH && digitalRead(inputPins[GREEN]) == LOW && digitalRead(inputPins[RED]) == LOW) {
          lastinput = LOW;
          status = RESET;
        }
        delay(50);
    } while (status != RESET);
  }
};
