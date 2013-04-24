/* TIC TAC TINO */

#include "tictactino.h"

void setup()
{
  tictactino::init(2, 3, 4, 5, 6);
}

void loop()
{
  tictactino::play();
  tictactino::show();
}
