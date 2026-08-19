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

#ifndef GTK_MAJOR_VERSION
#include <cmath>
#include <cstring>
#include <unordered_map>
#endif

const int NF = -1;
const bool DEBUG_MODE = NF != -1;

// allow any number of moves 0-3
const std::string fixed_field[] = {
    R"(00000011
11111010
11101001
11111010
11110010
00000000
01101011
11101011
001 111-111 010-111 111 111)",

    R"(11101101
00000110
00000000
00000110
11111011
10111110
00000000
10000011
1 1 1-01 01 11-11111
)",
    R"(
    00000000
    11101001 
    11100001 
    10000001 
    11010000 
    11100000 
    00110000 
    10100011 111 111 111 -
    1 -
    111 111 111 
    70_2
54_1
    )"};
static_assert(NF >= -1 && NF < int(std::size(fixed_field)));

using VInt = std::vector<int>;
using VVInt = std::vector<VInt>;
using VString = std::vector<std::string>;
using VPIntInt = std::vector<std::pair<int, int>>;

const int N = 8;
const int NT = 3;
const int ADD_INDEX = 1;
const int InvalidValue = -1;
const int ALL_COUNT = 39;
const int DOT_INDEX = ALL_COUNT - 1;
#ifdef GTKMM_MAJOR_VERSION
const std::string ARROW = "→";
#else
const std::string ARROW = "=";
#endif
const std::string ALWAYS_GAME_OVER = "always game over";

const std::unordered_map<std::string, std::string> MAP = {
    {"01 11 10", "z"}, {"01 11 01", "t"},   {"001 111", "l"},
    {"101 111", "π"},  {"01 11", "corner"}, {"001 001 111", "CORNER"}};

std::chrono::steady_clock::time_point gameBegin;
bool startFromEmptyField = 0;
VInt gfigureIndex;
int field[N][N];
std::set<uint32_t> set2;
std::string hs;
#ifdef USE_SKIPC
int skipc2;
#endif

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

double successProbability(int i) {
  return (1 - std::pow(1 - double(i) / ALL_COUNT, 3));
}

std::string possibleString(int i, const int o) {
  auto d = successProbability(i) * 100;
  if (o == 0)
    return std::format(" {} {:.0f}%", i, d);
  else if (o == 1)
    return std::format("possible {} {:.2f}%", i, d);
  else
    return std::format("{} {:.1f}%", i, d);
}

struct Figure {
  std::vector<std::array<int, 2>> xy; // position of filled squares
  VInt xfill, yfill;                  // filled squares axis
  int width, height;
  std::string code, mincode, name;

  void set(std::string _code, std::string _mincode, VVInt f) {
    int x, y;
    std::set<int> sx, sy;
    std::string s;
    bool all;
    code = _code;
    mincode = _mincode;
    width = f[0].size();
    height = f.size();
    for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
        if (f[y][x]) {
          xy.push_back({x, y});
          sx.insert(x);
          sy.insert(y);
        }
      }
    }
    xfill.assign(sx.begin(), sx.end());
    yfill.assign(sy.begin(), sy.end());

    // recognize lines, squares
    all = std::all_of(std::begin(f), std::end(f),
                      [](auto x) { return x.size() == 1; });
    if (all) {
      name = f.size() == 1 ? "dot" : std::to_string(f.size()) /*+ "V"*/;
    } else {
      all = std::all_of(std::begin(f), std::end(f), [](auto x) {
        return std::all_of(std::begin(x), std::end(x),
                           [](auto x) { return x == 1; });
      });
      if (all) {
        name = f.size() == 1 ? std::to_string(f[0].size()) /*+ "H"*/
                             : std::format("square{}", f.size());
      } else {
        try {
          name = MAP.at(mincode);
        } catch (const std::out_of_range &e) {
          name = "invalid name";
        }
      }
    }
  }

  std::string to_string() {
    std::string s;
    s = "xfill";
    for (auto &i : xfill) {
      s += ' ' + std::to_string(i);
    }
    s += " yfill";
    for (auto &i : yfill) {
      s += ' ' + std::to_string(i);
    }
    s += " xy";
    for (auto &i : xy) {
      s += std::format(" {}{}", i[0], i[1]);
    }
    return std::format("{} {} {}x{} {}", name, code, width, height, s);
    // return std::format("code{} mincode{} size{}{} {}", code, mincode,
    // width,
    //                    height, s);
  }
} ALL_FIGURES[ALL_COUNT];

Figure *findFigureIt(std::string code) {
  return std::find_if(std::begin(ALL_FIGURES), std::end(ALL_FIGURES),
                      [&code](auto &e) { return e.code == code; });
}

Figure &findFigure(std::string code) { return *findFigureIt(code); }

int findFigureIndex(std::string code) {
  auto it = findFigureIt(code);
  return std::distance(std::begin(ALL_FIGURES), it);
}

struct Info {
  int x, y, x1, y1, x2, y2, lines, field[N][N], n[3], nlines[3], prize_x,
      prize_y, fullestimate;
  bool end, prize_add;
  // field - field after move
  void operator=(const Info &e) {
    x = e.x;
    y = e.y;
    x1 = e.x1;
    y1 = e.y1;
    x2 = e.x2;
    y2 = e.y2;
    lines = e.lines;
    end = e.end;
    copy(e.field, field);
    std::copy(e.n, e.n + 3, n);
    std::copy(e.nlines, e.nlines + 3, nlines);
    prize_x = e.prize_x;
    prize_y = e.prize_y;
    prize_add = e.prize_add;
    fullestimate = e.fullestimate;
  }

  Info() {}
  Info(int _x, int _y, int _estimate, int _lines, bool _end, int _possibleAfter,
       int _field[N][N]) {
    x = _x;
    y = _y;
    lines = _lines;
    end = _end;
    copy(_field, field);
    setPrizeInvalid();
    fullestimate =
        (_possibleAfter * 100 + (64 - countFill(field))) * 100 + _estimate;
  }

  int getEstimate() { return fullestimate % 100; }

  int getFieldc() { return N * N - (fullestimate / 100) % 100; }

  int getPossibleAfter() { return fullestimate / 10000; }

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

  std::string movesString(int size) {
    std::string s;
    int i;
    for (i = 0; i < size; i++) {
      s += std::format(
          "{}{}{} {}\n",
          size == 1 ? "" : std::to_string(n[i] + ADD_INDEX) + " ", gx(i), gy(i),
          nlines[i] == 0 ? "" : '[' + std::to_string(nlines[i]) + ']');
    }
    return s;
  }

  std::string prizeString() {
    return std::format("{}{}{}{}{} {} {}", prize_add ? '+' : '-', prize_x,
                       prize_y, ARROW, getPossibleAfter(), getFieldc(),
                       getEstimate());
  }

  std::string to_string() {
    int pa = getPossibleAfter();
    return std::format(
        "{}{}{}{}{}{}{}", x, y, lines ? '[' + std::to_string(lines) + ']' : "",
        end ? "e" : "", pa == InvalidValue ? "" : possibleString(pa, 0), ARROW,
        getEstimate());
  }

  bool isInvalid() { return x == InvalidValue; }
  void setInvalid() { x = InvalidValue; }

  bool isPrizeInvalid() { return prize_x == InvalidValue; }
  void setPrizeInvalid() { prize_x = InvalidValue; }
  void setPrize(int x, int y, int add) {
    prize_x = x;
    prize_y = y;
    prize_add = add;
  }

  bool operator<(const Info &i) const { return fullestimate > i.fullestimate; }

} InvalidInfo, best;

using VInfo = std::vector<Info>;

struct Prev {
  std::string code, out[NT];
  Info best;
} previous[4];

#define PRINT(fmt, ...)                                                        \
  std::cout << std::format(fmt " {}:{}\n" __VA_OPT__(, )                       \
                               __VA_ARGS__ __VA_OPT__(, )                      \
                                   std::source_location::current()             \
                                       .file_name(),                           \
                           std::source_location::current().line());

template <typename... Args>
void print_line_helper(std::source_location loc, Args &&...args) {
  bool first = true;

  auto print_with_space = [&](auto &&arg) {
    if (!first) {
      std::cout << " ";
    }
    first = false;
    std::cout << std::forward<decltype(arg)>(arg);
  };

  (print_with_space(std::forward<Args>(args)), ...);

  std::cout << " " << loc.file_name() << ":" << loc.line() << "\n";
}

#define PRINT_LINE1(...)                                                       \
  print_line_helper(std::source_location::current() __VA_OPT__(, ) __VA_ARGS__);

// pr("123");
#define pr PRINT_LINE1
// pr1("error {} {}", v[i], v[i + 1]);
#define pr1 PRINT
#define pri PRINT_LINE1("")

int resetAndGetLines(int i, int j, const Figure &f, int fill[N][N],
                     int after[N][N]) {
  int l = 0, x, y;
  copy(fill, after);

  for (auto &_x : f.xfill) {
    x = _x + i;
    for (y = 0; y < N && fill[y][x]; y++)
      ;
    if (y == N) {
      for (y = 0; y < N; y++) {
        after[y][x] = 0;
      }
      l++;
    }
  }

  for (auto &_y : f.yfill) {
    y = _y + j;
    for (x = 0; x < N && fill[y][x]; x++)
      ;
    if (x == N) {
      for (x = 0; x < N; x++) {
        after[y][x] = 0;
      }
      l++;
    }
  }
  return l;
}

std::string join(const VString &vs, char sep = '\n', bool after = 0) {
  std::string s;
  bool f = 1;
  for (auto &a : vs) {
    if (f) {
      f = 0;
    } else {
      s += sep;
    }
    s += a;
  }
  if (after) {
    s += sep;
  }
  return s;
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

std::string to_string(const VVInt &a) {
  VString v;
  for (auto &x : a) {
    std::string s;
    for (auto &e : x) {
      s += std::to_string(e);
    }
    v.push_back(s);
  }
  return join(v, ' ');
}

// returns false if move impossible
bool make_move(int i, int j, const Figure &f, int field[N][N]) {
  int x, y, l;
  int fill[N][N], after[N][N];

  copy(field, fill);
  for (auto &xy : f.xy) {
    x = xy[0] + i;
    y = xy[1] + j;
    if (field[y][x]) {
      return false;
    }
    fill[y][x] = 1;
  }
  copy(fill, after);
  resetAndGetLines(i, j, f, fill, after);
  copy(after, field);
  return true;
}

void from_string(const std::string &st, int field[N][N]) {
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
    pr1("error {}", data);
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
    gfigureIndex[i] = findFigureIndex(v[i]);
  }

  for (; i < v.size(); i += 2) {
    b = make_move(v[i][0] - '0', v[i][1] - '0',
                  ALL_FIGURES[gfigureIndex[v[i + 1][0] - '1']], field);
    if (!b) {
      pr1("error {} {}", v[i], v[i + 1]);
      exit(1);
    }
  }
}

void parseInitialString() {
  if (DEBUG_MODE) {
    from_string(fixed_field[NF], field);
  } else {
    pr("error non debug mode");
  }
}

std::string gameTimeString() {
  auto elapsed1 = std::chrono::steady_clock::now() - gameBegin;
  auto duration_sec =
      std::chrono::duration_cast<std::chrono::seconds>(elapsed1).count();
  std::chrono::seconds sec{duration_sec};
  return std::format("time {:%T}{}\n", sec, startFromEmptyField ? "" : "*");
}

bool hasPossibleMoves(const Figure &f, const int field[N][N]) {
  int x, y;
  for (x = 0; x <= N - f.width; x++) {
    for (y = 0; y <= N - f.height; y++) {
      for (auto &xy : f.xy) {
        if (field[xy[1] + y][xy[0] + x]) {
          goto l99;
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
    i = 0;
    // j=0 square3,j=ALL_COUNT-1 dot
    for (j = 1; j < ALL_COUNT - 1; j++)
      if (hasPossibleMoves(ALL_FIGURES[j], field))
        i++;

    return i + 1;
  }
}

VInfo possibleMoves(const Figure &f, const VInt &recent,
                    const int field[N][N], bool fromEstimate = false) {
  int i, j, e, x, y, l, possible, fill[N][N], after[N][N];
  bool end;
  VInfo vi;
  for (j = 0; j <= N - f.height; j++) {
    for (i = 0; i <= N - f.width; i++) {
      e = 0;
      copy(field, fill);
      for (auto &xy : f.xy) {
        x = xy[0] + i;
        y = xy[1] + j;
        if (field[y][x]) {
          goto l183;
        }
        e += (x == 0 ? 1 : field[y][x - 1]) +
             (x == N - 1 ? 1 : field[y][x + 1]) +
             (y == 0 ? 1 : field[y - 1][x]) +
             (y == N - 1 ? 1 : field[y + 1][x]);
        fill[y][x] = 1;
      }

      l = resetAndGetLines(i, j, f, fill, after);
      if (!fromEstimate) {
        end = recent.empty() ? false
                             : !std::any_of(recent.begin(), recent.end(),
                                            [&after](auto &e) {
                                              return hasPossibleMoves(ALL_FIGURES[e], after);
                                            });
      }
      possible =
          !fromEstimate || recent.empty() ? countPossible(after) : InvalidValue;
      vi.push_back(Info(i, j, e, l, end, possible, after));
    l183:;
    }
  }
  return vi;
}

VInfo possibleMoves(const Figure &f, const int field[N][N]) {
  return possibleMoves(f, {}, field, true);
}

int index3(int i, int j) { return j + (i <= j); }

Info estimate(const VInt &vf, const int field[N][N], const VInt &figureIndex,
              const int code, const int lines) {
  Info r, e;
  VInt v2;
  int j, k;
  r.setInvalid();
  r.setPrizeInvalid();
  r.fullestimate = 0;
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
    auto v = possibleMoves(ALL_FIGURES[vf[i]], field);

    if (vf.size() == 1) {
      if (v.empty()) {
        return InvalidInfo;
      }
      auto it = std::min_element(v.begin(), v.end());
      it->setLines(0);
      return *it;
    }

    v2 = vf;
    v2.erase(v2.begin() + i);

    vfi = figureIndex;
    vfi.erase(vfi.begin() + i);

    for (auto &a : v) {
      j = (figureIndex[i] << 6) | (a.x << 3) | a.y; // 12bit
      if (vf.size() == 2 && lines == 0) {
        vc = {code, j};
        std::sort(vc.begin(), vc.end()); // asc

        j = 0;
        for (auto &a : vc) {
          j = (j << 12) | a;
        }

        if (set2.contains(j)) {
#ifdef USE_SKIPC
          skipc2++;
#endif
          continue;
        } else {
          set2.insert(j);
        }
      }
      e = estimate(v2, a.field, vfi, j, a.lines);

      if (e.isInvalid())
        continue;

      j = e.fullestimate + a.fullestimate % 100;
      if (r.fullestimate < j) {
        r.fullestimate = j;
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

bool same(const VInt &v, const int field[N][N]) {
  int j = 0;
  int t[N][N], a[N][N];
  for (auto &p : permutations(v.size())) {
    copy(field, t);
    for (auto &i : p) {
      if (!make_move(best.gx(i), best.gy(i), ALL_FIGURES[v[best.n[i]]], t)) {
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

VInt getNonEmptyIndex() {
  VInt vi;
  for (auto &i : gfigureIndex) {
    if (i != ALL_COUNT) {
      vi.push_back(i);
    }
  }
  return vi;
}

std::string nonEmptyFiguresString() {
  VString vs;
  for (auto a : getNonEmptyIndex()) {
    vs.push_back(ALL_FIGURES[a].code);
  }
  return join(vs, '-');
}

void findBest() {
  set2.clear();
#ifdef USE_SKIPC
  skipc2 = 0;
#endif
  best = estimate(getNonEmptyIndex(), field, gfigureIndex, 0, 0);
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
        make_move(i, j, ALL_FIGURES[DOT_INDEX], field);
      }

      findBest();
      if (!best.isInvalid()) {
        best.setPrize(i, j, !b);
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
  if (v.empty()) {
    return ALWAYS_GAME_OVER;
  }
  std::sort(v.begin(), v.end());
  for (auto &a : v) {
    s += a.prizeString() + "\n";
  }
  if (!v.empty()) {
    best = v[0];
  }
  return s;
}

std::string bestString() {
  std::string s;
  auto start = std::chrono::steady_clock::now();
  VInt v = getNonEmptyIndex();
  if (v.empty() || v.size() > 3)
    return "";

  s = to_string(field) + nonEmptyFiguresString();
  auto &prev = previous[v.size()];
  if (s == prev.code) {
    best = prev.best;
    return prev.out[1] + "\n" + gameTimeString();
  }
  prev.code = s;
  s = "";

  findBest();
  if (best.isInvalid()) {
    s = getPrizesString(); // also set best
  }
  if (best.isInvalid()) {
    s = ALWAYS_GAME_OVER + '\n'; //'\n' needs
  } else {
    if (v.size() == 1) {
      best.n[0] = 0;
    } else {
      s += std::string(same(v, field) ? "any order" : "order important") + "\n";
    }
    s += "after " + fillString(best.getFieldc()) + "\n" + "after " +
         possibleString(best.getPossibleAfter(), 1) + "\n" + "now " +
         fillString(countFill(field)) + "\n" + "now " +
         possibleString(countPossible(field), 1) + "\n";
  }
  auto end = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  s += std::format("time {}ms",
                   elapsed.count()); //+ (hs.empty() ? "" : '\n' + hs);
  prev.out[1] = s;                   // without game time
  s += '\n' + gameTimeString();

#ifdef USE_SKIPC
  // s += std::format("size {} {}", set2.size(), skipc2);
#endif
  prev.best = best;
  return s;
}

VVInt reverseX(const VVInt &matrix) {
  return matrix | std::views::transform([](auto &row) {
           return row | std::views::reverse | std::ranges::to<VInt>();
         }) |
         std::ranges::to<std::vector>();
}

VVInt reverseY(const VVInt &a) {
  return std::views::reverse(a) | std::ranges::to<std::vector>();
}

VVInt invertFigure(const VVInt &a, bool x, bool y) {
  VVInt b = x ? reverseX(a) : a;
  if (y) {
    b = reverseY(b);
  }
  return b;
}

VVInt rotate(const VVInt &matrix) {
  int rows = matrix.size();
  int cols = matrix[0].size();
  VVInt result(cols, VInt(rows));
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      result[j][rows - 1 - i] = matrix[i][j];
    }
  }
  return result;
}

void from_string(const std::string &s, VVInt &f) {
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

void init() {
  VString vs;
  int x, y, i, j;
  std::string s, key;
  std::set<std::string> set;

  /*first square3 & lines after, dot is last*/

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

  // dot
  vs.push_back("1");

  int k = 0;
  VVInt a;
  for (const auto &key : vs) {
    from_string(key, a);
    set.clear();
    auto r = rotate(a);
    for (x = 0; x < 2; x++) {
      for (y = 0; y < 2; y++) {
        for (i = 0; i < 2; i++) {
          VVInt f = invertFigure(i ? r : a, x, y);
          s = to_string(f);
          if (!set.contains(s)) {
            set.insert(s);
            ALL_FIGURES[k++].set(s, key, f);
          }
        }
      }
    }
  }

  InvalidInfo.setInvalid();
  gfigureIndex.resize(3);

  // for gtk newGame() do it
#ifndef GTKMM_MAJOR_VERSION
  gameBegin = std::chrono::steady_clock::now();
  best.setInvalid();
#endif
}

std::string toString(int t, char separator = ' ', int digits = 3) {
  // std::fixed to prevents scientific notation t=1234567.890123 b=1.23457e
  // +06
  std::stringstream c;
  c << std::fixed << t;
  std::string s, e, b = c.str();
  std::string::size_type p, p1;
  p = b.find('.');
  if (p != std::string::npos) {
    for (p1 = b.length() - 1; p1 > p && b[p1] == '0'; p1--)
      ;            //"3.875000"->"3.875"
    if (p != p1) { //"1.000" -> "1"
      e = b.substr(p, p1 - p + 1);
    }
    b = b.substr(0, p);
  }
  bool negative = std::is_signed<int>::value && t < 0;
  unsigned i = b.length() - 1;
  for (char a : b) {
    s += a;
    if (i % digits == 0 && i != 0 && (!negative || i != b.length() - 1)) {
      s += separator;
    }
    i--;
  }
  return s + e;
}
