#include "common.h"

/*
clear && g++ -std=c++23 console.cpp -o console.exe && ./console.exe
g++ console.cpp -o console.exe && ./console.exe
*/

int main() {
  int i;
  init();
  from_string(fixed_field[NF], field, figures);
  pr(bestString());
}
