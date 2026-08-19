#include "common.h"

/*
clear && g++ -std=c++23 console.cpp -o console.exe && ./console.exe
g++ console.cpp -o console.exe && ./console.exe
*/

int main() {
  int i;
  init();

  // for (auto &a : ALL_FIGURES) {
  //   pr(a.to_string());
  // }

  from_string(fixed_field[NF], field);
  for (auto a : gfigureIndex) {
    pr(ALL_FIGURES[a].name)
  }
  pr(to_string(field));
  // pr(bestString());
}
