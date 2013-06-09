/* TIC TAC TINO */

#include "tictactino.h"

/* Anschluss-Pins des Arduino */
int latchPin = 3;
int clockPin = 4;
int dataPin = 5;
int greenPin = 7;
int redPin = 8;

/* Angaben fuer die benutzerdefinierte Demo-Funktion
      - optional, folgende Zeilen koennen auskommentiert werden */
      
uint32_t matrix, matrix_array[8] = {       // Bitmuster fuer verschiedene
    0, 495, 0, 16, 0, 253440, 0, 8192  };  // Bilder / Muster
int democounter = 0;

void demo(uint32_t &data)
{
  data = matrix_array[democounter];
  ++democounter %= 8;
  delay(25);
}
/* Angaben fuer die benutzerdefinierte Demo-Funktion ENDE */

void setup()
{
  //matrix = 0;
  
  /* Board / Spiel initialisieren */
  tictactino::init(greenPin, redPin, dataPin, clockPin, latchPin, 45000);
  
  /* Uebergeben der benutzerdefinierten Demo-Funktion - optional, kann auskommentiert werden */
  tictactino::demo(demo, matrix);
}

void loop()
{
  /* Spiel starten */
  tictactino::play();
  
  /* Spielergebnis anzeigen */
  tictactino::show();
}
