/* ttttest */

#include "tictactino.h"


void setup()
{
  tictactino::init(4, 5, 6, 2, 3);
}

void loop()
{
  //tictactino::reset();
  tictactino::play();
  tictactino::show();
}
