#include "tictactino.h"

namespace tictactino {
  enum Player { GREEN, RED };
  enum Status { UNDEF, RESET, RUNNING, WIN_GREEN, WIN_RED, EQUAL, DEMO };
  enum Command { NOOP, MOVE, SET };
  const uint16_t winmask[8] = { 7, 56, 73, 84, 146, 273, 292, 448 };
  int inputPins[2], dPin, cPin, lPin, lastinput;
  uint8_t counter = 0;
  uint16_t playground[2], blinkmask, cursor, winpattern;
  uint32_t reg;
  unsigned long inputtimer, blinktimer;
  volatile Status status = UNDEF;
  boolean blinktoggle;
  void reset();
  void refresh();
  void turn(Player p);
  Command read(Player p);
  void move(Player p);
  void set(Player p);
  Player player();
  
  /*
    *
    * Board initialisieren
    * greenPin, redPin - digitale Eingaenge, an denen die Taster haengen
    * dataPin, clockPin, latchPin - digit. Ausgaenge, die drei 8 Bit Shift
    *                               Register steuern (74HC595)
  */
  void init(int greenPin, int redPin, int dataPin, int clockPin, int latchPin)
  {
    inputPins[GREEN] = greenPin;
    inputPins[RED] = redPin;
    dPin = dataPin;
    cPin = clockPin;
    lPin = latchPin;
    pinMode(lPin, OUTPUT);
    pinMode(dPin, OUTPUT);
    pinMode(cPin, OUTPUT);
    pinMode(inputPins[GREEN], INPUT);
    pinMode(inputPins[RED], INPUT);
  }
  
  /*
    *
    * Board fuer neues Spiel vorbereiten
    *
  */
  void reset()
  { 
    // blinkmask - Bitmaske fuer blinkende Darstellung
    blinkmask = 1;
    cursor = 1;
    // Bitmuster, das zum Sieg gefuehrt hat (nach Spielende gesetzt)
    winpattern = 0;
    lastinput = LOW;
    blinktoggle = false;
    // Spielfeld-Bitmap der einzelnen Spieler
    playground[GREEN] = 0;
    playground[RED] = 0;
    // Timer-Variablen, die Tasteneingabe und Matrix-Anzeige (blinken) steuern
    inputtimer = 0;
    blinktimer = 0;
    // Spielzugzaehler
    counter = 0;
    // Register - 24 (!!!) Bit Wort, das in die Shiftregister geschrieben wird
    // 2 Bit frei; 4 Bit Zugzaehler; 9 Bit Spielfeld rot; 9 Bit Spielfeld gruen
    reg = 0;
    blinktimer = millis() + blinkfreq;
    // Spielstatus
    status = RESET;
  }
  
  /*
    *
    * Gibt den aktuellen Spieler zurück
    *
  */
  Player player()
  {
    if ((counter & 1) == 0) return GREEN;
    else return RED;
  }

  /*
    *
    * Spielen
    *
  */
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
  
  /*
    *
    * Eingabe-Schleife - Wechselnde Abfrage der Spielerzuege
    * bis unentschieden oder Sieg erreicht ist
    *
  */
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
    refresh();
  }
  
  /*
    *
    * Einlesen der Taster-Eingabe - kurzer Tastendruck bewegt Cursor (blinkende LED)
    *                               langer Tastendruck setzt Spielzug
    *
  */
  Command read(Player p)
  {
    Command cmd = NOOP;
    int input = digitalRead(inputPins[p]);
    if (lastinput == LOW) { // Taster war nicht gedrückt
      // Tastendruck entdeckt
      if (input == HIGH) {
        lastinput = HIGH;
        inputtimer = millis() + 1000; // Timer starten
      }
      // nichts passiert
    }
    else if (input == LOW && inputtimer <= millis()) { // Taster war gedrueckt worden
      // nach langem Druck losgelassen -> Zug setzen
      cmd = SET;
      lastinput = LOW;
    }
    else if (input == LOW && inputtimer > millis()) {
      // nach kurzem Druck losgelassen - Cursor weiterbewegen
      cmd = MOVE;
      lastinput = LOW;
    }
    delay(50); // Taster entprellen
    return cmd;
  }
  
  /*
    *
    * Cursor auf naechste moegliche Position setzen
    *
  */
  void move(Player p)
  {
    uint16_t mask = (playground[GREEN] | playground[RED]);
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
    blinktimer = 0;
  }
  
  /*
    *
    * LED-Matrix auffrischen
    *
  */
  void refresh()
  {
    uint32_t mask = 0;
    byte data = 0;
    if (blinktimer <= millis()) { // Blink-Timer abgelaufen -> Matrix neu schreiben
      // Registervariable mit Spielstand und Zähler befüllen
      reg = ((uint32_t) counter << 18) | ((uint32_t) playground[RED] << 9) | ((uint32_t) playground[GREEN]);
      switch (status) {
        case RUNNING:
          mask = (uint32_t) blinkmask << (9 * (counter & 1)); // Waehrend des Spiels Cursor anzeigen
          break;
        case EQUAL:
          mask = reg; // Bei unentschieden alle Felder blinken lassen
          break;
        case WIN_RED:
          mask = (uint32_t) blinkmask << 9; // Sieg rot - rote Gewinnreihe blinken lassen
          break;
        case WIN_GREEN:
          mask = (uint32_t) blinkmask; // Sieg gruen - gruene Gewinnreihe blinken lassen
          break;
      }
      // blinkende LEDs ein- bzw. ausschalten
      if (blinktoggle == true) {
          reg |= mask;
          blinktoggle = false;
        }
        else {
          reg &= ~mask;
          blinktoggle = true;
        }
      
      // Registerinhalt in Matrix schreiben
      digitalWrite(lPin, LOW);
      for (int i = 2; i >= 0; --i) {
        data = (byte) ((reg >> (i * 8)) & 255);
        shiftOut(dPin, cPin, MSBFIRST, data);
      }
      digitalWrite(lPin, HIGH);
      blinktimer = millis() + blinkfreq;  
    }
    
  }
  
  /*
    *
    * Spielzug setzen und Spielsituation ueberpruefen
    *
  */
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
    }
    // liegt eine Gewinnsituation vor?
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
    if (winpattern == 0 && counter >= 8) { // kein weiterer Zug mehr moeglich und keine Gewinnsituation -> unentschieden
      if (counter == 8) {
        //playground[p] |= cursor;
        set(p);
        blinkmask = 0;
        cursor = 0;
        return;
      }
      else status = EQUAL;
    }
    blinktimer = 0;
  }
  
  /*
    *
    * Anzeige des Spieausgangs nach Spielende
    * Warten auf (beliebigen) Tastendruck, um neues Spiel zu starten
    *
  */
  void show()
  {
    cursor = 0;
    blinkmask = winpattern;
    do {
        refresh();
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
