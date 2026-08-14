#include "gtk.h"
/*
clear && g++ -std=c++23 e:/slovesno/gtk.cpp `pkg-config gtkmm-4.0 --cflags
--libs` -lgdi32 && ./a.exe clear && g++ -o gtk.exe -std=c++23
e:/slovesno/gtk.cpp `pkg-config gtkmm-4.0 --cflags --libs` -lgdi32 -mwindows &&
./gtk.exe

-mwindows - в консоль ничего не выводится

C:\msys64\home\user\

fill,time,possible coutn always even game over to best
*/

std::vector<Info> possibleMoves(const Figure &f, const VFigure &recent,
                                const int field[N][N]) {
  int i, j, es, x, y, k, l, _x, _y, end, fi, possible;
  int fill[N][N], after[N][N];
  std::vector<Info> ea;
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
      possible = fi == 0 ? countPossible(after) : -1;
      ea.push_back(Info(i, j, es, l, end, possible, after));
    l183:;
    }
  }
  return ea;
}

std::vector<Info> possibleMoves(const Figure &f, const int field[N][N]) {
  auto v = possibleMoves(f, {}, field);
  for (auto &a : v) {
    a.countEstimate();
  }
  return v;
}

int index3(int i, int j) { return j + (i <= j); }

Info estimate(const VFigure &vf, const int field[N][N]) {
  Info r, e;
  VFigure v2;
  int i, j, k;
  r.setInvalid();
  r.estimate = 0;
  for (i = 0; i < vf.size(); i++) {
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
    for (auto &a : v) {
      e = estimate(v2, a.field);
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
      } else {
      }
    }
  }
  return r;
}

struct Prev {
  std::string code, out;
  Info best;
} previous[4];

std::string bestString() {
  // printf("best\n");
  VFigure v;
  std::string s;
  int i, j;
  auto start = std::chrono::steady_clock::now();
  for (auto &a : figures) {
    if (!a.empty()) {
      v.push_back(a);
    }
  }

  if (v.size() >= 1 && v.size() <= 3) {
    bool hasDot = std::any_of(v.begin(), v.end(), [](const auto &a) {
      return a.size() == 1 && a[0].size() == 1;
    });

    if (hasDot && v.size() == 3 && countFill(field) < 10 /*15*/) {
      best.setInvalid();
      return "the field is too empty";
    }
    s = to_string(field) + to_string(v);
    // std::cout<<to_string(v)<<"\n";
    auto &prev = previous[v.size()];
    if (s == prev.code) {
      best = prev.best;
      return prev.out + "\nsame";
    }
    prev.code = s;

    best = estimate(v, field);
    if (best.isInvalid()) {
      s = "always game over\n";
    } else {
      if (v.size() == 1) {
        best.n[0] = 0;
      }
      s = "";
      for (i = 0; i < v.size(); i++) {
        s += best.ss(i, v.size() == 1) + "\n";
      }

      std::vector<int> vi; // estimate (64 - fieldc)   possibleAfter
      j = best.estimate;
      for (i = 0; i < 3; i++, j /= 100) {
        vi.push_back(j % 100);
      }

      std::vector<std::string> vs = {
          std::format("estimate {}", toString(best.estimate, ' ', 2)),
          std::format("after {}", possibleString(vi[2], 2)),
          fillString(N * N - vi[1], 0)};
      s += join(vs);
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    s += std::format("time {}ms", elapsed.count());
    prev.out = s;
    prev.best = best;
    return s;
  } else {
    return "";
  }
}
