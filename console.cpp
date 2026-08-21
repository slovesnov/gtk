  #include "common.h"

/*
clear && g++ -std=c++23 console.cpp -o console.exe && ./console.exe
g++ console.cpp -o console.exe && ./console.exe
*/

int main() {
  int i;

  init();

  parseInitialString();
  // for (auto a : gfigureIndex) {
  //   pr(ALL_FIGURES[a].name)
  // }
  pr(to_string(field));
  pr(bestString());
  pr(possibleSquare3(field));
  // if (!best.isInvalid()) {
  //   pr(best.movesString());
  //   pr(best.get(SCORE));
  // }
}
