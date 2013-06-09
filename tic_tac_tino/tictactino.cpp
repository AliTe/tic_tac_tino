#include "tictactino.h"

namespace tictactino {
  int blinkfreq = 250; // blinking frequency in ms - should be ~ >150
  unsigned long idletime;
  enum Player { GREEN, RED };
  enum Status { UNDEF, RESET, RUNNING, WIN_GREEN, WIN_RED, EQUAL, DEMO };
  enum Command { NOOP, MOVE, SET, ABORT, IDLELOOP };
  const uint16_t winmask[8] = { 
    7, 56, 73, 84, 146, 273, 292, 448   };
  int inputPins[2], dPin, cPin, lPin, lastinput;
  uint8_t counter = 0;
  uint16_t playground[2], blinkmask, cursor, winpattern;
  uint16_t demoground[2];
  uint32_t reg;
  unsigned long inputtimer, blinktimer, resettimer, idletimer;
  volatile Status status = UNDEF;
  boolean blinktoggle;
  boolean playertoggle;
  void reset();
  void refresh();
  void registerWrite();
  void turn(Player p);
  Command read(Player p);
  void move(Player p);
  void set(Player p);
  void demoloop();
  void defaultdemo(uint32_t &data);
  void (*demofunction)(uint32_t &data) = defaultdemo;
  uint32_t demodata = 1;
  Player player();

  /*
   *
   * Board initialisieren
   * greenPin, redPin - digitale Eingaenge, an denen die Taster haengen
   * dataPin, clockPin, latchPin - digit. Ausgaenge, die drei 8 Bit Shift
   *                               Register steuern (74HC595)
   */
  void init(int greenPin, int redPin, int dataPin, int clockPin, int latchPin, unsigned long idleTime)
  {
    inputPins[GREEN] = greenPin;
    inputPins[RED] = redPin;
    playertoggle = true;
    lastinput = LOW;
    dPin = dataPin;
    cPin = clockPin;
    lPin = latchPin;
    idletime = idleTime;
    pinMode(lPin, OUTPUT);
    pinMode(dPin, OUTPUT);
    pinMode(cPin, OUTPUT);
    pinMode(inputPins[GREEN], INPUT);
    pinMode(inputPins[RED], INPUT);
  }

  /*
   * Benutzerdefinierte Demo-Funktion
   * Argumente Funktionspointer, Datenpointer (32 bit Integer als Bitmaske
   *                                           Bits 0-8 -> gruenes Feld
   *                                           Bits 9-17 -> rotes Feld
   *  
   */
  void demo(void (*demo_function)(uint32_t &userdata), uint32_t &data)
  {
    demofunction = demo_function;
    demodata = data;
  }

  /*
   *
   * Board fuer neues Spiel vorbereiten
   *
   */
  void reset()
  { 
    // visuelle Anzeige eines RESETs
    while (lastinput == HIGH) {
      reg = 511;
      registerWrite();
      delay(100);
      reg <<= 9;
      registerWrite(); 
      delay(100);
      lastinput = digitalRead(inputPins[player()]);
    }
    // blinkmask - Bitmaske fuer blinkende Darstellung
    blinkmask = 1;
    // Bitmuster, das zum Sieg gefuehrt hat (nach Spielende gesetzt)
    winpattern = 0;
    lastinput = LOW;
    blinktoggle = false;
    // Spielfeld-Bitmap der einzelnen Spieler
    playground[GREEN] = 0;
    playground[RED] = 0;
    // Timer-Variablen, die Tasteneingabe und Matrix-Anzeige (blinken) steuern
    inputtimer = 0;
    resettimer = 0;
    // Spielzugzaehler
    counter = 0;
    // Register - 24 (!!!) Bit Wort, das in die Shiftregister geschrieben wird
    // 2 Bit frei; 4 Bit Zugzaehler; 9 Bit Spielfeld rot; 9 Bit Spielfeld gruen
    reg = 0;
    // Spieler wechseln
    playertoggle = !playertoggle;
    // Cursor setzen
    if (player() == GREEN) cursor = 256;
    else cursor = 0;
    move(player());
    // Timer starten
    blinktimer = millis() + blinkfreq;
    idletimer = millis() + idletime;
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
    if (playertoggle == false) {
      if ((counter & 1) == 0) return GREEN;
      else return RED;
    }
    else {
      if ((counter & 1) == 0) return RED;
      else return GREEN;
    }
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
    while (status == RUNNING || status == DEMO) {
      // enter game main loop
      turn(player());
      if (status == DEMO) demoloop();
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
    case IDLELOOP:
      status = DEMO;
      return;
      break;
      
    case MOVE:
      move(p);
      idletimer = millis() + idletime;
      break;

    case SET:
      set(p);
      idletimer = millis() + idletime;
      break;

    case ABORT:
      status = RESET;
      return;
      break;

    case NOOP:
      break;
    }
    refresh();
  }

  /*
   *
   * Einlesen der Taster-Eingabe - kurzer Tastendruck bewegt Cursor (blinkende LED)
   *                               langer Tastendruck (~1 sec.) setzt Spielzug
   *                               sehr langer Tastendruck (~4 sec.) fuehrt RESET aus
   *
   */
  Command read(Player p)
  {
    Command cmd = NOOP;
    int input = digitalRead(inputPins[p]);
    if (idletimer <= millis()) {
      return IDLELOOP;
    }
    else if (lastinput == LOW) { // Taster war nicht gedrückt
      // Tastendruck entdeckt
      if (input == HIGH) {
        lastinput = HIGH;
        inputtimer = millis() + 1000; // Input-Timer starten
        resettimer = inputtimer + 3000; // Reset-Timer starten
      }
      // nichts passiert
    }
    else if (resettimer <= millis()) { // Taster war gedrueckt worden
      // nach sehr langem Druck losgelassen -> Spieler gibt auf -> Reset
      cmd = ABORT;
      blinkfreq = 250;
    }
    else if (input == HIGH && inputtimer <= millis()) {
      // Inputtimer abgelaufen, Taste noch gedrueckt -> schneller blinken
      blinkfreq = 80;
    }
    else if (input == LOW && inputtimer <= millis()) {
      // nach langem Druck losgelassen -> Zug setzen
      cmd = SET;
      lastinput = LOW;
      blinkfreq = 250;
    }
    else if (input == LOW && inputtimer > millis()) {
      // nach kurzem Druck losgelassen - Cursor weiterbewegen
      cmd = MOVE;
      lastinput = LOW;
      blinkfreq = 250;
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
    if (blinktimer <= millis()) { // Blink-Timer abgelaufen -> Matrix neu schreiben
      // Registervariable mit Spielstand und Zähler befüllen
      reg = ((uint32_t) counter << 18) | ((uint32_t) playground[RED] << 9) | ((uint32_t) playground[GREEN]);
      switch (status) {
      case RUNNING:
        mask = (uint32_t) blinkmask << (9 * player()); // Waehrend des Spiels Cursor anzeigen
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
      if (blinktoggle == false) {
        reg |= mask;
        blinktoggle = true;
      }
      else {
        reg &= ~mask;
        blinktoggle = false;
      }

      registerWrite();
      blinktimer = millis() + blinkfreq;  
    }

  }
  
  /*
   *
   * DEMO-Loop
   * Schleife, in der die ueber demo() uebergebene 
   * benutzerdefinierte Funktion aufgerufen wird, bis Tastendruck erfolgt
   * 
   * Auf die benutzerdefinierte Demo-Funktion kann über demofunction Zugegriffen werden,
   * die anzuzeigenden Bitmasken liegen unter *demodata
   * Dabei sind die Bits 0..8 die gruene und Bits 9..17 die rote LED-Matrix; restliche Bits -> 0
   * 
   */
  void demoloop()
  {
    while (status == DEMO) {
      demofunction(demodata);
      reg = demodata & 262143; // nur die Bits 0..17 sind relevant !!!
      registerWrite();
      if (lastinput == LOW && (digitalRead(inputPins[GREEN]) == HIGH || digitalRead(inputPins[RED]) == HIGH))
        lastinput = HIGH;
      else if (lastinput == HIGH && digitalRead(inputPins[GREEN]) == LOW && digitalRead(inputPins[RED]) == LOW) {
        lastinput = LOW;
        inputtimer = millis() + 1000;
        resettimer = inputtimer + 3000;
        idletimer = inputtimer + idletime - 1000;
        status = RUNNING;
      }
      delay(50); // benoetigt zum entprellen der Taster
    }
  }
  
  /*
   *
   * Default-Demo-Funktion - wird benutzt, wenn keine beutzerdefinierte
   * Funktion uebergeben wurde
   *
  */
  void defaultdemo(uint32_t &data)
  {
    if (data <= 256) {
      // gruen
      data <<= 1;
      if (data > 256) data = 131072;
    }
    else {
      // rot
      data >>= 1;
      if (data <= 256) data = 1;
    }
    delay(25);
  }

  
  /*
   *
   * Schieberegister beschreiben
   *
   */
  void registerWrite()
  {
    // Registerinhalt in Matrix schreiben
    byte data = 0;
    digitalWrite(lPin, LOW);
    for (int i = 2; i >= 0; --i) {
      data = (byte) ((reg >> (i * 8)) & 255);
      shiftOut(dPin, cPin, MSBFIRST, data);
    }
    digitalWrite(lPin, HIGH);
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
   * Anzeige des Spielausgangs nach Spielende
   * Warten auf (beliebigen) Tastendruck, um neues Spiel zu starten
   *
   */
  void show()
  {
    if (status == RESET) { // RESET wurde vor Spielende aufgerufen-> nichts anzeigen
      return;
    }
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

