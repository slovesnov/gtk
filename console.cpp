#include "common.h"

/*
clear && g++ -std=c++23 console.cpp -o console.exe && ./console.exe
g++ console.cpp -o console.exe && ./console.exe
*/
static const int BITS[] = {6, 9, 8, 4};

template <size_t N>
constexpr std::array<int, N> calculate_suffix_sums(const int (&bits)[N]) {
  std::array<int, N> result{};
  for (size_t i = 1; i < N; ++i) {
    int sum = 0;
    for (size_t j = i; j < N; ++j) {
      sum += bits[j];
    }
    result[i-1] = sum;
  }
  result[N-1]=0;

  return result;
}

class MyClass {
public:
  inline static const int BITS[] = {6, 9, 8, 4};
  inline static const std::array<int, std::size(BITS)> SBITS = calculate_suffix_sums(BITS);
};

uint32_t fullestimate = 0;
int getP(int i) {
  return (fullestimate >> MyClass::SBITS[i]) & ((1 << BITS[i]) - 1);
}

int main() {

    for (int val : MyClass::SBITS) {
    std::cout << val << " ";
  }

  int v[] = {8, 7, 3, 2};
  int i;

  for (i = 0; i < std::size(v); i++) {
    fullestimate |= v[i] << MyClass::SBITS[i];
  }

  pr(std::format("{:b}", fullestimate));

  for (i = 0; i < 4; i++)
    pr(getP(i));

  //  1000000010 - 00000011 - 0100
}
/*
int main() {
  int i;
  init();


  for (auto &a : ALL_FIGURES) {
    if (a.squares() > 5) {
      pr(a.to_string());
    }
  }

  parseInitialString();
  for (auto a : gfigureIndex) {
    pr(ALL_FIGURES[a].name)
  }
  pr(to_string(field));
  pr(bestString());
  if (!best.isInvalid())
    pr(best.movesString())
}
*/