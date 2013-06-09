#ifndef TICTACTINO_H
#define TICTACTINO_H

#include "Arduino.h"

namespace tictactino {
  
  /*
   *
   * Board initialisieren
   *
   * greenPin    Eingang, an dem Taster von Spieler gruen liegt
   * redPin      Eingang, an dem Taster von Spieler rot liegt
   * dataPin     Ausgang an dem die Bitmaske der Spielfeldmatrix seriell in die
   *             Schieberegister gesendet wird
   * clockPin    Ausgang zur Steuerung der Schieberegister
   * latchPin    Ausagng zum Steuern der Schieberegister (Spielfeld-Matrix anzeigen)
   * idleTime    (optional) Dauer der Inaktivitaet, nach der der Demo-Loop aufgerufen wird (in ms, default 45 sec.)
   *
  */
  void init(int greenPin, int redPin, int dataPin, int clockPin, int latchPin, unsigned long idleTime = 45000);
  
  /*
   *
   * demo-Loop
   * 
   * Hier kann eine benutzerdefinierte Funktion uebergeben werden,
   * die nach 2 min. Inaktivität aufgerufen wird, um das Display zu gestalten.
   * Sie muss als Argument einen Verweis auf eine uit32_t Variable haben, die die Spielfeldmatrix enthaelt.
   * Bits 0..8 -> gruene Matrix; Bits 9..17 -> rote Matrix.
   * Alle uebrigen Bits werden auf 0 gesetzt
   *
  */
  void demo(void (*demo_function)(uint32_t &userdata), uint32_t &data);
  
  /*
   *
   * Die Spiel-Schleife betreten
   *
  */
  void play();
  
  /*
   *
   * Anzeige des Spielergebnisses
   *
   * Wird Angezeigt, bis beliebige Taste gedrueckt wird
   *
  */
  void show();
};

#endif
