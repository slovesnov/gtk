#include "common.h"

/*
clear && g++ -std=c++23 console.cpp -o console.exe && ./console.exe
g++ console.cpp -o console.exe && ./console.exe
*/

VInfo getPrizesInfo() {
  VInfo v;
  int i, j, original[N][N];
  bool b;
  copy(field, original);
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      b = field[j][i];
      if (b) {
        field[j][i] = 0;
      } else {
        copy(original, field);
        make_move(i, j, DOT, field);
      }

      findBest();
      if (!best.isInvalid()) {
        best.prize_x = i;
        best.prize_y = j;
        best.prize_add = !b;
        v.push_back(best);
      }

      if (b) {
        field[j][i] = 1;
      } else {
        copy(original, field);
      }
    }
  }
  return v;
}

int main() {
  int i;
  init();
  from_string(fixed_field[NF], field, figures);
  pr(to_string(field));
  auto v = getPrizesInfo();
  std::sort(v.begin(),v.end());

  for(auto&a:v){
    pr(a.prize_x,a.prize_y,a.prize_add,a.estimate)
  }

//   int f[N][N];

//   make_move(2, 0, DOT, field);
//   pr(to_string(field));
//   make_move(4, 0, DOT, field);
//   pr(to_string(field));

  /*   for (auto &a : v[0]) { // empty
    copy(field, f);
    //make_move()
      field[a.second][a.first] = 0;
      findBest();
      if (!best.isInvalid()) {
        pr(a.first, a.second, best.estimate)
      }
      field[a.second][a.first] = 1;
    }
   */

  /*   for (auto &a : v[1]) { // filled
      field[a.second][a.first] = 0;
      findBest();
      if (!best.isInvalid()) {
        pr(a.first, a.second, best.estimate)
      }
      field[a.second][a.first] = 1;
    }
   */

  //   for (auto a : v) {
  //     for (auto b : a) {
  //       pr(b.first, b.second);
  //     }
  //     pr("\n");
  //   }

  //   for (i = 0; i < 3; i++) {
  //     pr(to_string(figures[i]));
  //   }
}
