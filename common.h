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
    R"(11010010
10001110
11000000
00111010
00101010
00001110
00110100
11111100
11 10 11-11 10 10-111 111 111)",

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
using PIntInt = std::pair<int, int>;
using VPIntInt = std::vector<PIntInt>;

const int N = 8;
const int NT = 3;
const int ADD_INDEX = 1;
const int InvalidValue = -1;
const int ALL_COUNT = 39;
const int DOT_INDEX = ALL_COUNT - 1;
#ifdef GTK_MAJOR_VERSION
const std::string ARROW = "→";
#else
const std::string ARROW = "=";
#endif
const std::string ALWAYS_GAME_OVER = "always game over";

const std::unordered_map<std::string, std::string> MAP = {
    {"01 11 10", "z"},
    {"01 11 01", "t"},
    {"001 111", "l"},
    {"101 111",
#ifdef GTK_MAJOR_VERSION
     "π"
#else
     "n"
#endif
    },
    {"01 11", "corner"},
    {"001 001 111", "CORNER"}};

std::chrono::steady_clock::time_point gameBegin;
bool startFromEmptyField = 0;
VInt gfigureIndex;
bool field[N][N];
std::set<uint32_t> set2;
// std::string hs;
#ifdef USE_SKIPC
int skipc2;
#endif

std::string make_string_spaced() { return ""; }

template <typename First, typename... Args>
std::string make_string_spaced(const First &first, const Args &...args) {
  std::ostringstream ss;
  ss << first;
  ((ss << " " << args), ...);
  return ss.str();
}

template <typename... Args>
void print_line_helper(std::source_location loc, Args &&...args) {
  if constexpr (sizeof...(Args) > 0) {
    std::cout << make_string_spaced(args...) << " ";
  }
  std::cout << loc.file_name() << ":" << loc.line() << "\n";
}

// pr("123",i,v);
#define pr(...)                                                                \
  print_line_helper(std::source_location::current() __VA_OPT__(, ) __VA_ARGS__);

#define pri pr()

// pr1("error {} {}", v[i], v[i + 1]);
#define pr1(fmt, ...)                                                          \
  std::cout << std::format(fmt " {}:{}\n" __VA_OPT__(, )                       \
                               __VA_ARGS__ __VA_OPT__(, )                      \
                                   std::source_location::current()             \
                                       .file_name(),                           \
                           std::source_location::current().line());

bool same(const bool f1[N][N], const bool f2[N][N]) {
  return std::equal(&f1[0][0], &f1[0][0] + N * N, &f2[0][0]);
}

void copy(const bool src[N][N], bool dest[N][N]) {
  std::copy(&src[0][0], &src[0][0] + N * N, &dest[0][0]);
}

int countFill(const bool field[N][N]) {
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
  return 1 - std::pow(1 - double(i) / ALL_COUNT, 3);
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

struct MakeMoveResult {
  bool valid;
  int score;
};

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

  int squares() const { return xy.size(); }

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
    return std::format("{} {} {}x{} sq{} {}", name, code, width, height,
                       squares(), s);
    // return std::format("code{} mincode{} size{}{} {}", code, mincode,
    // width,
    //                    height, s);
  }
} ALL_FIGURES[ALL_COUNT];

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

int findFigureIndex(std::string code) {
  auto it = std::find_if(std::begin(ALL_FIGURES), std::end(ALL_FIGURES),
                         [&code](auto &e) { return e.code == code; });
  return std::distance(std::begin(ALL_FIGURES), it);
}

bool hasPossibleMoves(const Figure &f, const bool field[N][N]) {
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

bool possibleRectange(bool i, bool j, const bool field[N][N], int w, int h) {
  int x, y;
  for (x = 0; x < w; x++) {
    for (y = 0; y < h; y++) {
      if (field[y + j][x + i])
        return false;
    }
  }
  return true;
}

bool possibleRectange(const bool field[N][N], int w, int h) {
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

int countPossible(const bool field[N][N]) {
  int i, j;
  bool square3 = possibleRectange(field, 3, 3), h5, v5;
  if (square3) {
    h5 = possibleRectange(field, 5, 1);
    v5 = possibleRectange(field, 1, 5);
    return ALL_COUNT - 2 + (h5 ? 1 : possibleRectange(field, 4, 1) - 1) +
           (v5 ? 1 : possibleRectange(field, 1, 4) - 1);
  } else {
    i = 1;
    // j=0 square3, j=ALL_COUNT-1 dot
    for (j = 1; j < ALL_COUNT - 1; j++)
      if (hasPossibleMoves(ALL_FIGURES[j], field))
        i++;

    return i;
  }
}

#define FIELDS_FIRST

#ifdef FIELDS_FIRST
enum { POSSIBLE_AFTER, FIELDS, SCORE, ESTIMATE };
#else
enum { POSSIBLE_AFTER, SCORE, FIELDS, ESTIMATE };
#endif

struct Info {
  int x, y, x1, y1, x2, y2, lines, n[3], nlines[3], prize_x, prize_y;
  bool field[N][N], end, prize_add;
  uint32_t fullestimate;
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

  /*Note on change order need change constants POSSIBLE_AFTER,
  FIELDS, SCORE, ESTIMATE and Params v = {_possibleAfter, N * N -
  countFill(field), linesScore.second,  _estimate};
  - possibleAfter 1-39, 6bits
  - 64-fieldC 0-64, 8bits
  - score (can be summed) 1- 9+360=369 9bits
  - estimate (can be summed) for figure with <=5 dots <=3*5, for square3 (only
  figure with >5 dots) 4corners*2+4*1=10 5bits, for 3 figures suppose<=32
  */
  static const int TOTAL_PARAMS = 4;
  static const int POSSIBLE_AFTER_BITS = 6;
  static const int FIELDS_BITS = 8;
  static const int SCORE_BITS = 9;
  static const int ESTIMATE_BITS = 5;

  using Params = std::array<int, TOTAL_PARAMS>;
  inline static constexpr Params BITS = []() {
    Params arr{};
#define A(p) arr[p] = p##_BITS;
    A(POSSIBLE_AFTER)
    A(FIELDS)
    A(SCORE)
    A(ESTIMATE)
#undef A
    return arr;
  }();

  inline static constexpr Params SBITS = []() {
    Params arr{};
    for (int i = 1; i < TOTAL_PARAMS; ++i) {
      int sum = 0;
      for (int j = i; j < TOTAL_PARAMS; ++j) {
        sum += BITS[j];
      }
      arr[i - 1] = sum;
    }
    arr[TOTAL_PARAMS - 1] = 0;

    return arr;
  }();

  Info() {}
  Info(int _x, int _y, int estimate, PIntInt linesScore, bool _end,
       bool _field[N][N]) {
    x = _x;
    y = _y;
    end = _end;
    lines = linesScore.first;
    copy(_field, field);
    setPrizeInvalid();
    int possibleAfter = countPossible(field);

    Params v = {
#ifdef FIELDS_FIRST
        possibleAfter, N * N - countFill(field), linesScore.second, estimate
#else
        possibleAfter, linesScore.second, N * N - countFill(field), estimate
#endif

    };
    fullestimate = countEstimate(v);
  }

  static uint32_t countEstimate(Params v) {
    int i, e = 0;
    for (i = 0; i < TOTAL_PARAMS; i++) {
      e |= v[i] << SBITS[i];
    }
    return e;
  }

  Params get() const {
    Params v;
    auto e = fullestimate;
    for (int i = 3; i >= 0; i--) {
      v[i] = e & ((1 << BITS[i]) - 1);
      e >>= BITS[i];
    }
    return v;
  }

  int get(int i) const {
    int j = (fullestimate >> SBITS[i]) & ((1 << BITS[i]) - 1);
    if (i == FIELDS) {
      j = N * N - j;
    }
    return j;
  }

  uint32_t addFullEstimate(const Info &p) const {
    auto t = get();
    auto m = p.get();
    for (int a : {SCORE, ESTIMATE}) {
      if (t[a] + m[a] < 1 << BITS[a]) {
        t[a] += m[a];
      } else {
        pr1("error {} {} {} {}", a, t[a], m[a], 1 << BITS[a]);
        t[a] = (1 << BITS[a]) - 1;
      }
    }
    return countEstimate(t);
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

  std::string movesString() {
    std::string s;
    int i;
    int size = getNonEmptyIndex().size();
    for (i = 0; i < size; i++) {
      s += std::format(
          "{}{}{} {}\n",
          size == 1 ? "" : std::to_string(n[i] + ADD_INDEX) + " ", gx(i), gy(i),
          nlines[i] == 0 ? "" : '[' + std::to_string(nlines[i]) + ']');
    }
    return s;
  }

  int totalLines() const {
    int size = getNonEmptyIndex().size();
    int i, j = 0;
    for (i = 0; i < size; i++) {
      j += nlines[i];
    }
    return j;
  }

  std::string prizeString() {
    return std::format("{}{}{}{}{} {} {}", prize_add ? '+' : '-', prize_x,
                       prize_y, ARROW, get(POSSIBLE_AFTER), get(FIELDS),
                       get(ESTIMATE));
  }

  std::string to_string() {
    return std::format("{}{}{}{}{}{}{}", x, y,
                       lines ? '[' + std::to_string(lines) + ']' : "",
                       end ? "e" : "", possibleString(get(POSSIBLE_AFTER), 0),
                       ARROW, get(ESTIMATE));
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

PIntInt resetGetLinesScore(int i, int j, const Figure &f, bool fill[N][N],
                           bool after[N][N]) {
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
  return {l, f.squares() + l * l * 10};
}

std::string to_string(const bool field[N][N]) {
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

MakeMoveResult make_move(int i, int j, const int findex, bool field[N][N]) {
  int x, y;
  bool fill[N][N], after[N][N];
  const Figure &f = ALL_FIGURES[findex];

  copy(field, fill);
  for (auto &xy : f.xy) {
    x = xy[0] + i;
    y = xy[1] + j;
    if (field[y][x]) {
      return {false, 0};
    }
    fill[y][x] = 1;
  }
  copy(fill, after);
  auto p = resetGetLinesScore(i, j, f, fill, after);
  copy(after, field);
  return {true, p.second};
}

void from_string(const std::string &st, bool field[N][N]) {
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

  for (i = 1; i < int(matches.size()); i++) {
    s = matches[i];
    if (s.empty()) {
      break;
    }
    v.push_back(s);
  }

  for (i = 0; i < 3; i++) {
    gfigureIndex[i] = findFigureIndex(v[i]);
  }

  for (; i < int(v.size()); i += 2) {
    b = make_move(v[i][0] - '0', v[i][1] - '0', gfigureIndex[v[i + 1][0] - '1'],
                  field)
            .valid;
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

VInfo possibleMoves(int findex, const VInt &recent, const bool field[N][N]) {
  int i, j, e, x, y;
  bool fill[N][N], after[N][N], end;
  PIntInt p;
  VInfo vi;
  const Figure &f = ALL_FIGURES[findex];
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
        // e += (x == 0 ? 0 : field[y][x - 1]) +
        //      (x == N - 1 ? 0 : field[y][x + 1]) +
        //      (y == 0 ? 0 : field[y - 1][x]) +
        //      (y == N - 1 ? 0 : field[y + 1][x]);
        // e += (x == 0 ? 1 : 2*field[y][x - 1]) +
        //      (x == N - 1 ? 1 : 2*field[y][x + 1]) +
        //      (y == 0 ? 1 : 2*field[y - 1][x]) +
        //      (y == N - 1 ? 1 : 2*field[y + 1][x]);
        e += (x == 0 ? 1 : field[y][x - 1]) +
             (x == N - 1 ? 1 : field[y][x + 1]) +
             (y == 0 ? 1 : field[y - 1][x]) +
             (y == N - 1 ? 1 : field[y + 1][x]);
        fill[y][x] = 1;
      }

      p = resetGetLinesScore(i, j, f, fill, after);
      end = recent.empty()
                ? false
                : !std::any_of(recent.begin(), recent.end(), [&after](auto &e) {
                    return hasPossibleMoves(ALL_FIGURES[e], after);
                  });
      vi.push_back(Info(i, j, e, p, end, after));
    l183:;
    }
  }
  return vi;
}

int index3(int i, int j) { return j + (i <= j); }

Info estimate(const VInt &vf, const bool field[N][N], const VInt &figureIndex,
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
    auto v = possibleMoves(vf[i], {}, field);

    if (vf.size() == 1) {
      if (v.empty())
        return InvalidInfo;
      r = *std::min_element(v.begin(), v.end());
      r.setLines(0);
      return r;
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

      uint32_t est = e.addFullEstimate(a);
      if (r.fullestimate < est) {
        r.fullestimate = est;
        a.setLines(0);
        r.eq(0, i, a);
        if (vf.size() == 3) {
          for (k = 2; k > 0; k--) { // order is important
            e.nlines[k] = e.nlines[k - 1];
            r.eq(k, index3(i, e.n[k - 1]), e, k == 2);
          }
        } else {
          e.setLines(1);
          r.eq(1, 1 - i, e);
        }
      }
    }
  }
  return r;
}

VVInt permutations(int n) {
  VVInt r;
  VInt arr(n);
  std::iota(arr.begin(), arr.end(), 0);
  do {
    r.push_back(arr);
  } while (std::next_permutation(arr.begin(), arr.end()));
  return r;
}

int same(const VInt &v, const bool field[N][N]) {
  int j = 0, score, score0;
  bool sameScore = 1;
  bool t[N][N], a[N][N];
  for (auto &p : permutations(v.size())) {
    copy(field, t);
    score = 0;
    for (auto &i : p) {
      auto r = make_move(best.gx(i), best.gy(i), v[best.n[i]], t);
      if (!r.valid) {
        return 0;
      }
      score += r.score;
    }
    if (j) {
      if (!same(t, a)) {
        return 0;
      }
      if (score0 != score) {
        sameScore = 0;
      }
    } else {
      score0 = score;
      copy(t, a);
    }
    j++;
  }
  return sameScore ? 1 : 2;
}

std::string fillString(int i) {
  return std::format("fill {}/{}={:.2f}%", i, N * N, i * 100. / N / N);
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
  int i, j;
  bool b, original[N][N];
  copy(field, original);
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      b = field[j][i];
      if (b) {
        field[j][i] = 0;
      } else {
        copy(original, field);
        make_move(i, j, DOT_INDEX, field);
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

// make any first move non end game and try use parize after first move
VInfo getPrizesInfo1() {
  VInfo v;
  Info info;
  int i, orig;
  bool original[N][N];
  auto ne = getNonEmptyIndex();
  // todo skip same
  for (i = 0; i < 3; i++) {
    if (gfigureIndex[i] == ALL_COUNT) {
      continue;
    }
    orig = gfigureIndex[i];
    gfigureIndex[i] = ALL_COUNT;

    auto v = possibleMoves(orig, getNonEmptyIndex(), field);
    for (auto &p : v) {
      if (!p.end) {
        copy(field, original);
        make_move(p.x, p.y, orig, field);
        findBest();
        if (!best.isInvalid()) { // todo
          info.x = p.x;
          info.y = p.y;
          info.n[0] = i;
          v.push_back(info);
        }
        copy(original, field);
      }
    }
    gfigureIndex[i] = orig;
  }
  return v;
}

std::string getPrizesString() {
  std::string s;
  auto v = getPrizesInfo();
  if (v.empty()) {
    v = getPrizesInfo1();
    if (v.empty())
      return ALWAYS_GAME_OVER;
    else {
      // seems never happens
      s = "first move and prize after\n";
      for (auto &a : v) {
        s += std::format("{}{}={}\n", a.x, a.y, a.n[0]);
      }
      return s;
    }
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
  int i;
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
      const std::string ss[] = {"order important", "any order",
                                "different score"};
      s += ss[same(v, field)] + "\n";
    }
    std::string v[] = {fillString(best.get(FIELDS)),
                       possibleString(best.get(POSSIBLE_AFTER), 1),
                       fillString(countFill(field)),
                       possibleString(countPossible(field), 1),
                       std::to_string(best.get(SCORE)),
                       std::to_string(best.totalLines())};
    std::string v1[] = {"after", "after", "now", "now", "score", "lines"};
    for (i = 0; i < int(std::size(v)); i++) {
      s += v1[i] + " " + v[i] + "\n";
    }
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
  std::string s, k;
  std::set<std::string> set;
  VVInt a, f;

  /*first square3, dot is last*/
  // square
  for (i = 3; i >= 2; i--) {
    k = s = std::string(i, '1');
    for (j = 0; j < i - 1; j++) {
      k += ' ' + s;
    }
    vs.push_back(k);
  }

  // ugly
  for (const auto &[k, value] : MAP) {
    vs.push_back(k);
  }

  // line and dot, dot should be last
  for (i = 5; i > 0; i--) {
    k = std::string(i, '1');
    vs.push_back(k);
  }

  j = 0;
  for (const auto &k : vs) {
    from_string(k, a);
    set.clear();
    auto r = rotate(a);
    for (x = 0; x < 2; x++) {
      for (y = 0; y < 2; y++) {
        for (i = 0; i < 2; i++) {
          f = invertFigure(i ? r : a, x, y);
          s = to_string(f);
          if (!set.contains(s)) {
            set.insert(s);
            ALL_FIGURES[j++].set(s, k, f);
          }
        }
      }
    }
  }

  InvalidInfo.setInvalid();
  gfigureIndex.resize(3);

  // for gtk newGame() do it
#ifndef GTK_MAJOR_VERSION
  gameBegin = std::chrono::steady_clock::now();
  best.setInvalid();
#endif
}

std::string toString(int number, char separator = ' ', int digits = 3) {
  std::string s = std::to_string(number);
  int start_idx = (number < 0) ? 1 : 0;
  for (int i = s.length() - digits; i > start_idx; i -= digits) {
    s.insert(i, 1, separator);
  }
  return s;
}
