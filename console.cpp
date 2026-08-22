#include "common.h"

/*
clear && g++ -std=c++23 console.cpp -o console.exe && ./console.exe
g++ -std=c++23 console.cpp -o console.exe && ./console.exe
*/

int main() {
  init();
  parseInitialString();
  // for (auto a : gfigureIndex) {
  //   pr(ALL_FIGURES[a].name)
  // }

  bool original[N][N];
  copy(field, original);

  pr1("{}",gfigureIndex);
  pr(to_string(field));

  pr(bestString());

  if (best.isInvalid()) {
    auto v=getPrizesInfo1();
    pr(v.size());
  }

  pr1("{}",gfigureIndex);
  pr(same(field,original));
  //pr(to_string(field));

  int i=3;
  prv(1,"xyz",i)
}
