#include <algorithm>
#include <chrono>
#include <format>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <print>
#include <set>
#include <source_location>
#include <regex>

#ifdef GTK_MAJOR_VERSION
#else
#include <cmath>
#include <cstring>
#include <unordered_map>
#endif

const int NF = -1;
const bool DEBUG_MODE = NF != -1;

// allow any number of moves 0-3
const std::string fixed_field[] = {
    R"(
11010111
11111110
01001110
11101100
11111110
01110111
00000010
01111001
1 1 1 1 1-01 01 11-111 101
)",
    R"(00000000
    11101001 11100001 10000001 11010000 11100000 00110000 10100011 111 111 111 -
    1 -
    111 111 111 
    70_2
54_1
    )"};
static_assert(NF >= -1 && NF < int(std::size(fixed_field)));

using VInt = std::vector<int>;
using Figure = std::vector<VInt>;
using VFigure = std::vector<Figure>;
using VString = std::vector<std::string>;
using VPIntInt = std::vector<std::pair<int, int>>;

const int N = 8;
const int NT = 3;
const int ADD_INDEX = 1;
const int InvalidValue = -1;
const int ALL_COUNT = 39;
const Figure DOT = {{1}};
#ifdef GTKMM_MAJOR_VERSION
const std::string ARROW = "→";
#else
const std::string ARROW = "=";
#endif

std::chrono::steady_clock::time_point gameBegin;
bool startFromEmptyField = 0;
VInt figureIndex;
int field[N][N];
Figure figures[3];
std::set<uint32_t> set2;
int skipc2;

bool same(int f1[N][N], int f2[N][N]) {
  return std::equal(&f1[0][0], &f1[0][0] + N * N, &f2[0][0]);
}

void copy(const int src[N][N], int dest[N][N]) {
  std::copy(&src[0][0], &src[0][0] + N * N, &dest[0][0]);
}

int countFill(const int field[N][N]) {
  int i, j, c = 0;
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      if (field[j][i])
        c++;
    }
  }
  return c;
}

std::string possibleString(int i, const int o) {
  auto d = (1 - std::pow(1 - double(i) / ALL_COUNT, 3)) * 100;
  if (o == 0)
    return std::format(" {} {:.0f}%", i, d);
  else if (o == 1)
    return std::format("possible {} {:.2f}%", i, d);
  else
    return std::format("{} {:.1f}%", i, d);
}

struct HFigure {
  Figure figure;
  std::string string;
} ALL_EXCEPT_DOT[ALL_COUNT - 1];

struct Info {
  int x, y, x1, y1, x2, y2, estimate, lines, end, possibleAfter, field[N][N],
      fieldc, n[3], nlines[3], prize_x, prize_y;
  bool prize_add;
  // field,fieldc - field after move
  void operator=(const Info &e) {
    x = e.x;
    y = e.y;
    x1 = e.x1;
    y1 = e.y1;
    x2 = e.x2;
    y2 = e.y2;
    estimate = e.estimate;
    lines = e.lines;
    end = e.end;
    possibleAfter = e.possibleAfter;
    copy(e.field, field);
    fieldc = e.fieldc;
    std::copy(e.n, e.n + 3, n);
    std::copy(e.nlines, e.nlines + 3, nlines);
    prize_x = e.prize_x;
    prize_y = e.prize_y;
    prize_add = e.prize_add;
  }

  void setLines(int i) { nlines[i] = lines; }

  int &gx(int index) { return index == 0 ? x : (index == 1 ? x1 : x2); }

  int &gy(int index) { return index == 0 ? y : (index == 1 ? y1 : y2); }

  void eq(int index, int i, const Info &e, int j = 0) {
    int _x = j == 0 ? e.x : e.x1, _y = j == 0 ? e.y : e.y1;
    gx(index) = _x;
    gy(index) = _y;
    n[index] = i;
    nlines[index] = e.nlines[index];
  }

  std::string ss(int index, int o = 0) {
    return std::format(
        "{}{}{}{}", gx(index), gy(index),
        nlines[index] == 0 ? "" : '[' + std::to_string(nlines[index]) + ']',
        o ? "" : '_' + std::to_string(n[index] + ADD_INDEX));
  }

  Info() {}
  Info(int _x, int _y, int _estimate, int _lines, int _end, int _possibleAfter,
       int _field[N][N]) {
    x = _x;
    y = _y;
    estimate = _estimate;
    lines = _lines;
    end = _end;
    possibleAfter = _possibleAfter;
    copy(_field, field);
    fieldc = countFill(field);
    // endgame estimate
    estimate = (possibleAfter * 100 + (64 - fieldc)) * 100 + estimate;
  }

  bool isInvalid() { return x == InvalidValue; }
  void setInvalid() { x = InvalidValue; }

  std::string to_string() {
    return std::format(
        "{}{}{}{}{}{}{}", x, y, lines ? '[' + std::to_string(lines) + ']' : "",
        end ? "e" : "",
        possibleAfter == InvalidValue ? "" : possibleString(possibleAfter, 0),
        ARROW, estimate);
  }
  bool operator<(const Info &i) const { return estimate > i.estimate; }

} InvalidInfo, best;
using VInfo = std::vector<Info>;

struct Prev {
  std::string code, out[NT];
  Info best;
} previous[4];

const std::unordered_map<std::string, std::string> MAP = {
    {"01 11 10", "z"}, {"01 11 01", "t"},   {"001 111", "l"},
    {"101 111", "π"},  {"01 11", "corner"}, {"001 001 111", "CORNER"}};

/* #define PRINT(fmt, ...) \
  std::cout << std::format(fmt " line{}\n" __VA_OPT__(, )                      \
                               __VA_ARGS__ __VA_OPT__(, )                      \
                                   std::source_location::current()             \
                                       .line());
 */
#define PRINT(fmt, ...)                                                        \
  std::cout << std::format(fmt " {}:{}\n" __VA_OPT__(, )                       \
                               __VA_ARGS__ __VA_OPT__(, )                      \
                                   std::source_location::current()             \
                                       .file_name(),                           \
                           std::source_location::current().line());

// Вспомогательная функция вывода через стабильный std::cout
template <typename... Args>
void print_line_helper(std::source_location loc, Args &&...args) {
  bool first = true;

  // Лямбда-функция для вывода одного аргумента с пробелом
  auto print_with_space = [&](auto &&arg) {
    if (!first) {
      std::cout
          << " "; // Добавляем пробел перед каждым элементом, кроме первого
    }
    first = false;
    std::cout << std::forward<decltype(arg)>(arg);
  };

  // Раскрываем все переданные аргументы (Fold Expression)
  (print_with_space(std::forward<Args>(args)), ...);

  // В самом конце выводим файл, строку и перенос строки
  std::cout << " " << loc.file_name() << ":" << loc.line() << "\n";
}

// Макрос скрывает вызов функции определения строки кода
#define PRINT_LINE1(...)                                                       \
  print_line_helper(std::source_location::current() __VA_OPT__(, ) __VA_ARGS__);

// pr("123");
#define pr PRINT_LINE1
// pr1("error {} {}", v[i], v[i + 1]);
#define pr1 PRINT
#define pri PRINT_LINE1("")

void from_string(const std::string &s, Figure &f) {
  VInt v;
  f.clear();
  for (auto &a : s) {
    if (strchr("01", a)) {
      v.push_back(a - '0');
    } else {
      f.push_back(v);
      v.clear();
    }
  }
  f.push_back(v);
}

// returns false if move impossible
bool make_move(int i, int j, const Figure &f, int field[N][N]) {
  int _x, _y, x, y, l;
  int fill[N][N], after[N][N];
  std::set<int> xa, ya;

  copy(field, fill);
  for (_y = 0; _y < f.size(); _y++) {
    for (_x = 0; _x < f[_y].size(); _x++) {
      if (f[_y][_x]) {
        x = _x + i;
        y = _y + j;
        if (field[y][x]) {
          return false;
        }
        xa.insert(x);
        ya.insert(y);
        fill[y][x] = 1;
      }
    }
  }
  copy(fill, after);

  l = 0;
  for (auto &x : xa) {
    for (y = 0; y < N && fill[y][x]; y++)
      ;
    if (y == N) {
      for (y = 0; y < N; y++) {
        after[y][x] = 0;
      }
      l++;
    }
  }

  for (auto &y : ya) {
    for (x = 0; x < N && fill[y][x]; x++)
      ;
    if (x == N) {
      for (x = 0; x < N; x++) {
        after[y][x] = 0;
      }
      l++;
    }
  }
  copy(after, field);
  return true;
}

std::string to_string(const int field[N][N]) {
  std::string s;
  int i, j;
  for (j = 0; j < N; j++) {
    for (i = 0; i < N; i++) {
      s += std::to_string(field[j][i]);
    }
    s += "\n";
  }
  return s;
}

std::string to_string(const Figure &a, int o = 1) {
  std::string s;
  bool f = 1;
  for (auto &x : a) {
    if (f) {
      f = 0;
    } else {
      s += o ? ' ' : '\n';
    }
    for (auto &e : x) {
      s += std::to_string(e);
    }
  }
  return s;
}

std::string to_string(const VFigure &vf) {
  std::string s;
  bool f = 1;
  for (auto &a : vf) {
    if (f) {
      f = 0;
    } else {
      s += '-';
    }
    s += to_string(a);
  }
  return s;
}

void from_string(const std::string &st, int field[N][N], Figure figures[3]) {
  int i = 0, j = -1;
  bool b;
  VString v;
  std::string s, s1, s2, s3, data;
  for (auto &a : st) {
    j++;
    if (strchr("01", a)) {
      field[i / N][i % N] = a - '0';
      if (++i == N * N) {
        break;
      }
    }
  }
  data = st.substr(j + 1);
  s1 = R"(([01][01 ]*[01]|1))";
  s2 = "\\s*-\\s*";
  s3 = R"((?:\s+(\d{2})(?:\[\d+\])?_(\d))?)";
  s = s1 + s2 + s1 + s2 + s1 + s3 + s3 + s3;

  std::regex pattern(s);
  std::smatch matches;

  if (!std::regex_search(data, matches, pattern)) {
    if (DEBUG_MODE) {
      pr1("error {}", data);

    } else {
      pr("error non debug mode");
    }
    exit(1);
  }

  for (i = 1; i < matches.size(); i++) {
    s = matches[i];
    if (s.empty()) {
      break;
    }
    v.push_back(s);
  }

  for (i = 0; i < 3; i++) {
    from_string(v[i], figures[i]);
  }

  for (; i < v.size(); i += 2) {
    b = make_move(v[i][0] - '0', v[i][1] - '0', figures[v[i + 1][0] - '1'],
                  field);
    if (!b) {
      pr1("error {} {}", v[i], v[i + 1]);
      exit(1);
    }
  }
  /*     if (moves){ // just view moves
          for (i = 0; i < 3; i++){
              figures[i].clear();
          }
      }
   */
}

std::string gameTimeString() {
  auto elapsed1 = std::chrono::steady_clock::now() - gameBegin;
  auto duration_sec =
      std::chrono::duration_cast<std::chrono::seconds>(elapsed1).count();
  std::chrono::seconds sec{duration_sec};
  return std::format("time {:%T}{}\n", sec, startFromEmptyField ? "" : "*");
}

bool hasPossibleMoves(const Figure &f, const int field[N][N]) {
  int i, j, x, y;
  for (j = 0; j <= N - f.size(); j++) {
    for (i = 0; i <= N - f[0].size(); i++) {
      for (y = 0; y < f.size(); y++) {
        for (x = 0; x < f[y].size(); x++) {
          if (f[y][x] && field[y + j][x + i]) {
            goto l99;
          }
        }
      }
      return true;
    l99:;
    }
  }
  return false;
}

bool possibleRectange(int i, int j, const int field[N][N], int w, int h) {
  int x, y;
  for (x = 0; x < w; x++) {
    for (y = 0; y < h; y++) {
      if (field[y + j][x + i])
        return false;
    }
  }
  return true;
}

bool possibleRectange(const int field[N][N], int w, int h) {
  int x, y;
  for (x = 0; x <= N - w; x++) {
    for (y = 0; y <= N - h; y++) {
      if (possibleRectange(x, y, field, w, h)) {
        return true;
      }
    }
  }
  return false;
}

bool possibleSquare3(const int field[N][N]) {
  return possibleRectange(field, 3, 3);
}

bool possibleV(const int field[N][N], int n) {
  return possibleRectange(field, 1, n);
}

bool possibleH(const int field[N][N], int n) {
  return possibleRectange(field, n, 1);
}

int countPossible(const int field[N][N]) {
  int i, j;
  bool square3 = possibleSquare3(field), h5, v5;
  if (square3) {
    h5 = possibleV(field, 5);
    v5 = possibleH(field, 5);
    return ALL_COUNT - 2 + (h5 ? 1 : possibleH(field, 4) - 1) +
           (v5 ? 1 : possibleV(field, 4) - 1);
  } else {
    i = 0, j = -1;
    for (auto &e : ALL_EXCEPT_DOT) {
      j++;
      if (j && hasPossibleMoves(e.figure, field)) {
        i++;
      }
    }
    return i + 1;
  }
}

VInfo possibleMoves(const Figure &f, const VFigure &recent,
                    const int field[N][N], bool fromEstimate = false) {
  int i, j, es, x, y, k, l, _x, _y, end, fi, possible;
  int fill[N][N], after[N][N];
  VInfo ea;
  std::set<int> xa, ya;
  for (j = 0; j <= N - f.size(); j++) {
    for (i = 0; i <= N - f[0].size(); i++) {
      es = 0;
      copy(field, fill);
      xa.clear();
      ya.clear();
      for (_y = 0; _y < f.size(); _y++) {
        for (_x = 0; _x < f[_y].size(); _x++) {
          if (f[_y][_x]) {
            x = _x + i;
            y = _y + j;
            if (field[y][x]) {
              goto l183;
            }
            xa.insert(x);
            ya.insert(y);
            es += (x == 0 ? 1 : field[y][x - 1]) +
                  (x == N - 1 ? 1 : field[y][x + 1]) +
                  (y == 0 ? 1 : field[y - 1][x]) +
                  (y == N - 1 ? 1 : field[y + 1][x]);
            fill[y][x] = 1;
          }
        }
      }
      copy(fill, after);

      l = 0;
      for (auto &x : xa) {
        for (y = 0; y < N && fill[y][x]; y++)
          ;
        if (y == N) {
          for (y = 0; y < N; y++) {
            after[y][x] = 0;
          }
          l++;
        }
      }

      for (auto &y : ya) {
        for (x = 0; x < N && fill[y][x]; x++)
          ;
        if (x == N) {
          for (x = 0; x < N; x++) {
            after[y][x] = 0;
          }
          l++;
        }
      }

      fi = 0;
      end = recent.empty() ? 0 : 1;
      for (auto &e : recent) {
        fi++;
        if (end && hasPossibleMoves(e, after)) {
          end = 0;
        }
      }
      possible = !fromEstimate || fi == 0 ? countPossible(after) : InvalidValue;
      ea.push_back(Info(i, j, es, l, end, possible, after));
    l183:;
    }
  }
  return ea;
}

VInfo possibleMoves(const Figure &f, const int field[N][N]) {
  return possibleMoves(f, {}, field, true);
}

int index3(int i, int j) { return j + (i <= j); }

Info estimate(const VFigure &vf, const int field[N][N], const VInt &figureIndex,
              const int code, const int lines) {
  Info r, e;
  VFigure v2;
  int j, k;
  r.setInvalid();
  r.estimate = 0;
  VInt vi = {0}, vfi, vc;
  if (vf.size() > 1) { // skip same figures
    if (figureIndex[1] != figureIndex[0])
      vi.push_back(1);
    if (vf.size() > 2) {
      if (figureIndex[2] != figureIndex[1] && figureIndex[2] != figureIndex[0])
        vi.push_back(2);
    }
  }

  for (auto &i : vi) {
    auto v = possibleMoves(vf[i], field);

    if (vf.size() == 1) {
      if (v.empty()) {
        return InvalidInfo;
      }
      auto it = std::max_element(v.begin(), v.end(), [](auto &a, auto &b) {
        return a.estimate < b.estimate;
      });
      it->setLines(0);
      return *it;
    }

    v2 = vf;
    v2.erase(v2.begin() + i);

    vfi = figureIndex;
    vfi.erase(vfi.begin() + i);

    for (auto &a : v) {
      j = (figureIndex[i] << 6) | (a.x << 3) | a.y; // 12bit
      if (vf.size() == 2 /* && a.lines == 0 */ && lines == 0) {
        vc = {code, j};
        std::sort(vc.begin(), vc.end()); // asc

        j = 0;
        for (auto &a : vc) {
          j = (j << 12) | a;
        }

        if (set2.contains(j)) {
          skipc2++;
          continue;
        } else {
          set2.insert(j);
        }
      }
      e = estimate(v2, a.field, vfi, j, a.lines);

      if (e.isInvalid())
        continue;

      j = e.estimate + a.estimate % 100;
      if (r.estimate < j) {
        r.estimate = j;
        if (vf.size() == 3) {
          a.setLines(0);
          r.eq(0, i, a);
          for (int k = 2; k > 0; k--) { // order is important
            e.nlines[k] = e.nlines[k - 1];
            r.eq(k, index3(i, e.n[k - 1]), e, k == 2);
          }
        } else {
          a.setLines(0);
          e.setLines(1);
          r.eq(0, i, a);
          r.eq(1, 1 - i, e);
        }
      }
    }
  }
  return r;
}

std::vector<VInt> permutations(int n) {
  std::vector<VInt> r;
  std::vector<int> arr(n);
  std::iota(arr.begin(), arr.end(), 0);
  do {
    r.push_back(arr);
  } while (std::next_permutation(arr.begin(), arr.end()));
  return r;
}

bool same(const VFigure &v, const int field[N][N]) {
  int j = 0;
  int t[N][N], a[N][N];
  for (auto &p : permutations(v.size())) {
    copy(field, t);
    for (auto &i : p) {
      if (!make_move(best.gx(i), best.gy(i), v[best.n[i]], t)) {
        return false;
      }
    }
    if (j) {
      if (!same(t, a)) {
        return false;
      }
    } else {
      copy(t, a);
    }
    j++;
  }
  return true;
}

std::string fillString(int i) {
  return std::format("fill {}/{}={:.2f}%", i, N * N, i * 100. / N / N);
}

std::string join(const VString &vs) {
  std::string s;
  for (auto &a : vs) {
    s += a + '\n';
  }
  return s;
}

/* std::string uncode(int i) {
  // (figureIndex[i] << 6) | (a.x << 3) | a.y; // 12bit
  int y = i & 7;
  i >>= 3;
  int x = i & 7;
  i >>= 3;
  return std::format(
      "x{} y{} {}\n", x, y,
      i == ALL_COUNT - 1
          ? "dot"
          : (i >= ALL_COUNT ? std::to_string(i) : ALL_EXCEPT_DOT[i].string));
}
 */

void findBest() {
  VFigure v;
  std::string s;
  for (auto &a : figures) {
    if (!a.empty()) {
      v.push_back(a);
    }
  }

  figureIndex.clear();
  set2.clear();
  skipc2 = 0;
  for (auto &a : figures) {
    s = to_string(a);
    auto it = std::find_if(std::begin(ALL_EXCEPT_DOT), std::end(ALL_EXCEPT_DOT),
                           [&s](auto &e) { return e.string == s; });

    figureIndex.push_back(it == std::end(ALL_EXCEPT_DOT)
                              ? ALL_COUNT - 1
                              : std::distance(std::begin(ALL_EXCEPT_DOT), it));
  }
  best = estimate(v, field, figureIndex, 0, 0);
}

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

std::string getPrizesString() {
  std::string s;
  auto v = getPrizesInfo();
  std::sort(v.begin(), v.end());
  for (auto &a : v) {
    s += std::format("{}{}{}{}{}\n", a.prize_add ? '+' : '-', a.prize_x,
                     a.prize_y, ARROW, a.estimate);
  }
  if (!v.empty()) {
    best = v[0];
  }
  return s;
}

std::string bestString() {
  VFigure v;
  std::string s, so;
  int i, j;
  auto start = std::chrono::steady_clock::now();
  for (auto &a : figures) {
    if (!a.empty()) {
      v.push_back(a);
    }
  }

  if (v.size() >= 1 && v.size() <= 3) {
    s = to_string(field) + to_string(v);
    auto &prev = previous[v.size()];
    if (s == prev.code) {
      best = prev.best;
      return prev.out[1] + "\n" + gameTimeString();
    }
    prev.code = s;
    s = "";

    // figureIndex.clear();
    // set2.clear();
    // skipc2 = 0;
    // for (auto &a : figures) {
    //   s = to_string(a);
    //   auto it =
    //       std::find_if(std::begin(ALL_EXCEPT_DOT), std::end(ALL_EXCEPT_DOT),
    //                    [&s](auto &e) { return e.string == s; });

    //   figureIndex.push_back(
    //       it == std::end(ALL_EXCEPT_DOT)
    //           ? ALL_COUNT - 1
    //           : std::distance(std::begin(ALL_EXCEPT_DOT), it));
    // }

    // best = estimate(v, field, figureIndex, 0, 0);
    findBest();
    if (best.isInvalid()) {
      s = getPrizesString(); // aslo set best
    }
    if (best.isInvalid()) {
      s = "always game over";
    } else {
      if (v.size() == 1) {
        best.n[0] = 0;
      } else {
        s += std::string(same(v, field) ? "any order" : "order important") +
             "\n";
      }
      for (i = 0; i < v.size(); i++) {
        s += best.ss(i, v.size() == 1) + "\n";
      }

      VInt vi; // estimate (64 - fieldc)   possibleAfter
      j = best.estimate;
      for (i = 0; i < 3; i++, j /= 100) {
        vi.push_back(j % 100);
      }

      std::vector<std::string> vs = {fillString(N * N - vi[1]),
                                     possibleString(vi[2], 1)};
      s += join(vs);
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    s += std::format("time {}ms", elapsed.count());
    prev.out[1] = s; // without game time
    s += "\n"+gameTimeString();
    // s += std::format("size {} {}", set2.size(), skipc2);
    prev.best = best;
    return s;
  } else {
    return "";
  }
}

Figure reverseX(const Figure &matrix) {
  return matrix |
         std::views::transform([](auto &row) {
           // Разворачиваем строку и сразу превращаем её в VInt
           return row | std::views::reverse | std::ranges::to<VInt>();
         })
         // Превращаем весь внешний результат в
         // std::vector<VInt>
         | std::ranges::to<std::vector>();
}

Figure reverseY(Figure &a) {
  return std::views::reverse(a) | std::ranges::to<std::vector>();
}

Figure invertFigure(const Figure &a, bool x, bool y) {
  Figure b = x ? reverseX(a) : a;
  if (y) {
    b = reverseY(b);
  }
  return b;
}

Figure rotate(const Figure &matrix) {
  if (matrix.empty())
    return {};

  int rows = matrix.size();
  int cols = matrix[0].size();

  // Создаем новую матрицу с перевернутыми размерами (cols x rows)
  Figure result(cols, VInt(rows));

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      // Формула для поворота по часовой стрелке
      result[j][rows - 1 - i] = matrix[i][j];
    }
  }
  return result;
}

void init() {
  VString vs;
  int x, y, i, j;
  std::string s, key;
  std::set<std::string> set;

  /*first square3 & lines5 for possible figures after, dot is ignored*/

  // square
  for (i = 3; i >= 2; i--) {
    key = s = std::string(i, '1');
    for (j = 0; j < i - 1; j++) {
      key += ' ' + s;
    }
    vs.push_back(key);

    if (i == 3) {
      // line
      for (j = 5; j >= 2; j--) {
        key = std::string(j, '1');
        vs.push_back(key);
      }
    }
  }

  // ugly
  for (const auto &[key, value] : MAP) {
    vs.push_back(key);
  }

  int k = 0;
  Figure a;
  for (const auto &key : vs) {
    from_string(key, a);
    set.clear();
    auto r = rotate(a);
    for (x = 0; x < 2; x++) {
      for (y = 0; y < 2; y++) {
        for (i = 0; i < 2; i++) {
          auto f = invertFigure(i ? r : a, x, y);
          s = to_string(f);
          if (!set.contains(s)) {
            set.insert(s);
            ALL_EXCEPT_DOT[k++] = {f, s};
          }
        }
      }
    }
  }

  InvalidInfo.setInvalid();

  // for gtk newGame() do it
#ifndef GTKMM_MAJOR_VERSION
  gameBegin = std::chrono::steady_clock::now();
  best.setInvalid();
#endif
}
