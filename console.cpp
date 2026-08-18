#include "common.h"

/*
clear && g++ -std=c++23 console.cpp -o console.exe && ./console.exe
g++ console.cpp -o console.exe && ./console.exe
*/

std::vector<VPIntInt> getEmptyFillPoints(const int field[N][N]) {
  std::vector<VPIntInt> v;
  v.resize(2);
  int i, j;
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      v[field[j][i]].push_back({i, j});
    }
  }
  return v;
}

int main() {
  int i;
  init();
  from_string(fixed_field[NF], field, figures);
  pr1(to_string(field));
  auto v = getEmptyFillPoints(field);

//   int f[N][N];
//   copy(field,f)
  for (auto& a : v[1]) {//filled
    field[a.second][a.first]=0;

    

    field[a.second][a.first]=1;
  }

//   for (auto a : v) {
//     for (auto b : a) {
//       pr1(b.first, b.second);
//     }
//     pr1("\n");
//   }

  //   for (i = 0; i < 3; i++) {
  //     pr1(to_string(figures[i]));
  //   }
  pr1(best.isInvalid()) pr1(bestString());
  //   std::cout<<std::format("{}",) ;
}
