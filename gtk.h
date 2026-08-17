#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <gdkmm/general.h>
#include <gdkmm/pixbuf.h>
#include <glibmm.h>
#include <glibmm/main.h>
#include <gtkmm/application.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/styleprovider.h>
#include <gtkmm/textview.h>
#include <gtkmm/window.h>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>
#include <windows.h>

using VString = std::vector<std::string>;
using VInt = std::vector<int>;
using Figure = std::vector<VInt>;
using VFigure = std::vector<Figure>;
using VPIntInt = std::vector<std::pair<int, int>>;

int timer = 1;
int saveText = 0;
const bool LOG = 1;
const int TIMER_MILLISECONDS = 500; // 800
const int SAVE_TIMER_MILLISECONDS = 3000;
const int NT = 3;
const int N = 8;
const std::string LOG_FILE = "log.txt";
const std::string SCREEN_DIR = "png";

const int NF = -1;
const bool DEBUG_MODE = NF != -1;

const std::string fixed_field[] = {R"(01011001
11011101
00111000
00111111
00000000
10011101
10010100
00001000
101 111-111 111 111-10 11
02[1]_3
23_1
52_2
)",

                                   R"(00010000
01011110
00011111
00000000
11100011
11010111
11010111
10010110
10 11 01-111 111 111-111 111 111)",

                                   R"(00000000
    11101001 11100001 10000001 11010000 11100000 00110000 10100011 111 111 111 -
    1 -
    111 111 111 )"};

static_assert(NF >= -1 && NF < int(std::size(fixed_field)));
const std::unordered_map<std::string, std::string> MAP = {
    {"01 11 10", "z"}, {"01 11 01", "t"},   {"001 111", "l"},
    {"101 111", "π"},  {"01 11", "corner"}, {"001 001 111", "CORNER"}};
const int DX = 763 - 852;
const int DY = 347 - 202;
const int DX1 = 743 - 763;
const int DY1 = 780 - 347;
const int STEPS = 24;
const int SX = 147;
const int STEP = 52;
const int SMALL_SQUARE_SIZE = 100;
const int DRAW_AREA_SQUARE = 21;
cairo_rectangle_int_t picture_rectangle;
// ABGR
const std::vector<uint32_t> FC[] = {
    {0xffab2578}, {0xff59ed9e, 0xff45dcf7, 0xff7676ff, 0xffffb945, 0xfff65ae9}};
const uint32_t EMPTY[] = {0xff9c2469, 0xff952463, 0xff8e245c, 0xff872355,
                          0xff7f224d, 0xff782247, 0xff702240, 0xff692139};
const uint32_t POSSIBLE_COLOR[] = {0xff59ed9e, 0xffffb945, 0xff45dcf7,
                                   0xff45dcf7, 0xff7676ff, 0xfff65ae9};

uint32_t BG_COLOR = 0xE6D8AD;
// default color fo rebug mode
uint32_t figure_color[] = {0xff59ed9e, 0xffffb945, 0xff45dcf7};

const int ADD_INDEX = 1;
const std::string SAVE_PNG = "save png";
const std::string SAVE_TEXT = "add text to log";
int gtotalWidth, field[N][N];
Figure figures[3];
VInt figureIndex;
uint32_t *gp;
std::vector<uint8_t> gbuffer;
std::chrono::steady_clock::time_point gameBegin;
const int ALL_COUNT = 39;
const int InvalidValue = -1;
bool startFromEmptyField = 0;
VPIntInt fillStatistics, possibleStatistics;

void copy(const int source[N][N], int dest[N][N]);
std::string possibleString(int possible, int o);
std::string fillString(int possible);
int countPossible(const int field[N][N]);
int countFill(const int field[N][N]);
std::string join(const VString &vs);
std::string dateTimeString(int o = 0);
std::string timeString() { return dateTimeString(1); }
std::string possibleStatString();
std::string fillStatString();
void from_string(const std::string &s, int field[N][N], Figure figures[3]);
void from_string(const std::string &s, Figure &f);
std::string toString(int t, char separator = ' ', int digits = 3);
std::string savePng();
Figure rotate(const Figure &matrix);
Figure invertFigure(const Figure &a, bool x, bool y);

struct HFigure {
  Figure figure;
  std::string string;
} ALL_EXCEPT_DOT[ALL_COUNT - 1];

struct Info {
  int x, y, x1, y1, x2, y2, estimate, lines, end, possibleAfter, field[N][N],
      fieldc, n[3], nlines[3];
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
  }
  Info(int _x, int _y, int _estimate, int _lines, int _end) {
    x = _x;
    y = _y;
    estimate = _estimate;
    lines = _lines;
    end = _end;
  }

  bool isInvalid() { return x == InvalidValue; }
  void setInvalid() { x = InvalidValue; }

  std::string to_string() {
    return std::format(
        "{}{}{}{}{}→{} ", x, y, lines ? '[' + std::to_string(lines) + ']' : "",
        end ? "e" : "",
        possibleAfter == InvalidValue ? "" : possibleString(possibleAfter, 0),
        estimate);
  }
  bool operator<(const Info &i) const {
    if (end != i.end) {
      return end < i.end;
    }
    if (possibleAfter != i.possibleAfter) {
      return possibleAfter > i.possibleAfter;
    }

    if (estimate != i.estimate) {
      return estimate > i.estimate;
    }

    return lines > i.lines;
  }

  void countEstimate() {
    // endgame estimate
    estimate = (possibleAfter * 100 + (64 - fieldc)) * 100 + estimate;
  }
};
Info InvalidInfo, best;

struct PointInfo {
  int x;
  int y;
  uint32_t color;
};

struct Prev {
  std::string code, out[NT];
  Info best;
} previous[4];

struct FigureStatistics {
  std::string show, mincode;
  std::vector<std::pair<std::string, int>> v;
  int total, squares;

  FigureStatistics(std::string _show, std::string _mincode, std::string _code) {
    show = _show;
    mincode = _mincode;
    v.push_back({_code, 1});
  }
  void count() {
    total = 0;
    for (auto &e : v) {
      total += e.second;
    }
    squares = std::count(mincode.begin(), mincode.end(), '1') * total;
  }
  bool operator<(const FigureStatistics &e) const { return total > e.total; }
  std::string to_string(int _total, int _max) const {
    std::string s;
    if (v.size() != 1) {
      for (auto const &e : v) {
        s += " " + std::to_string(e.second);
      }
    }
    return std::format("{} {:{}} {:.1f}%{}\n", show, total,
                       static_cast<int>(std::log10(_max)) + 1,
                       total * 100. / _total, s);
  }
};

std::string get_screenshot_winapi();
std::vector<Info> possibleMoves(const Figure &f, const VFigure &recent,
                                const int field[N][N]);
std::string toABGR(uint32_t c, bool onlyRGB = true);
std::string code(const Figure &a);
std::string to_string(const int field[N][N]);
std::string to_string(const VFigure &vf);
std::string to_string(const Figure &a, int o = 1);
std::string bestString();

class MyWindow : public Gtk::Window {
public:
  MyWindow()
      : m_buttonSave(SAVE_PNG), m_buttonSaveText(SAVE_TEXT), m_buttonTimer() {
    int i;

    set_resizable(false);
    m_title = std::format("gtkmm {}.{}.{}", GTKMM_MAJOR_VERSION,
                          GTKMM_MINOR_VERSION, GTKMM_MICRO_VERSION);
    set_title(m_title);
    set_icon_name("app-icon");
    for (i = 0; i < NT; i++) {
      if (i == 2) {
        m_text_view[i].set_valign(Gtk::Align::START);
      } else {
        m_scrolled_window[i].set_child(m_text_view[i]);
        m_scrolled_window[i].set_policy(Gtk::PolicyType::AUTOMATIC,
                                        Gtk::PolicyType::AUTOMATIC);
        if (i == 0)
          m_scrolled_window[i].set_size_request(200, 700);
        else {
          m_scrolled_window[i].set_vexpand(true);
        }
      }

      std::string css_data = std::format(R"(textview {{
        font-family: 'Times New Roman';
        font-size: 14px;
      }})");
      auto css_provider = Gtk::CssProvider::create();
      css_provider->load_from_data(css_data);
      m_text_view[i].get_style_context()->add_provider(
          css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      m_text_view[i].set_wrap_mode(Gtk::WrapMode::WORD);
    }

    auto box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 2);
    box->append(m_buttonSave);
    box->append(m_buttonSaveText);
    box->append(m_buttonTimer);
    box->append(m_text_view[2]);
    box->append(m_drawing_area);
    box->append(m_scrolled_window[1]);

    auto box1 = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
    box1->append(m_scrolled_window[0]);
    box1->append(*box);
    set_child(*box1);

    m_drawing_area.set_valign(Gtk::Align::START);
    m_drawing_area.set_vexpand(false);
    m_drawing_area.set_content_width(DRAW_AREA_SQUARE * N);
    m_drawing_area.set_content_height(DRAW_AREA_SQUARE * N);
    m_drawing_area.set_draw_func(sigc::mem_fun(*this, &MyWindow::on_draw));

    m_buttonSave.signal_clicked().connect(
        [this]() { updateSaveButton(savePng()); });

    m_buttonSaveText.signal_clicked().connect([this]() {
      saveText = 1;
      m_buttonSaveText.set_label("waiting...");
    });

    m_buttonTimer.signal_clicked().connect([this]() {
      timer = !timer;
      setButtonTimerText();
    });

    setButtonTimerText();
    init();

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &MyWindow::tick),
                                   TIMER_MILLISECONDS);
  }

  void newGame() {
    gameBegin = std::chrono::steady_clock::now();
    best.setInvalid();
    fillStatistics.clear();
    possibleStatistics.clear();
    m_figureStatistics.clear();
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

    if (LOG)
      std::filesystem::create_directories("./" + SCREEN_DIR);

    InvalidInfo.setInvalid();
    newGame();
  }

  void setColor(const Cairo::RefPtr<Cairo::Context> &cr, int i) {
    double b = ((i >> 16) & 0xFF) / 255.0;
    double g = ((i >> 8) & 0xFF) / 255.0;
    double r = (i & 0xFF) / 255.0;
    cr->set_source_rgb(r, g, b);
  }

  void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height) {

    int i, j, n, x, y, cx, cy;
    const int Q = DRAW_AREA_SQUARE;

    if (best.isInvalid() && !DEBUG_MODE) {
      auto style_context = get_style_context();
      Gdk::RGBA bg_color;
      style_context->lookup_color("theme_bg_color", bg_color);
      Gdk::Cairo::set_source_rgba(cr, bg_color);
      cr->rectangle(0, 0, width, height);
      cr->fill();
    } else {
      VFigure vf;
      for (auto &f : figures) {
        if (!f.empty()) {
          vf.push_back(f);
        }
      }

      VInt nf[N][N];
      setColor(cr, BG_COLOR);
      for (y = 0; y < N; y++) {
        for (x = 0; x < N; x++) {
          if (field[y][x])
            cr->rectangle(x * Q, y * Q, Q, Q);
        }
      }
      cr->fill();

      const int BITS = 4;
      const int MB = 1 << BITS;
      const int MASK = MB - 1;

      for (n = 0; n < vf.size(); n++) {
        i = best.gx(n);
        j = best.gy(n);
        if (i != InvalidValue) {
          auto f = vf[best.n[n]];
          for (y = 0; y < f.size(); y++) {
            for (x = 0; x < f[y].size(); x++) {
              if (f[y][x]) {
                auto &v = nf[j + y][i + x];
                v.push_back(n);
                if (field[j + y][i + x] &&
                    !std::ranges::contains(v, BG_COLOR)) {
                  v.push_back(BG_COLOR);
                }
              }
            }
          }
        }
      }

      for (y = 0; y < N; y++) {
        for (x = 0; x < N; x++) {
          auto &n = nf[y][x];
          if (n.size()) {
            cx = x * Q;
            cy = y * Q;
            double center_x = cx + Q / 2.0;
            double center_y = cy + Q / 2.0;
            double radius = Q * G_SQRT2 / 2;
            double angle_step = 2 * G_PI / n.size();
            cr->save();
            cr->rectangle(cx, cy, Q, Q);
            cr->clip();
            for (i = 0; i < n.size(); i++) {
              double start_angle = i * angle_step + G_PI / 4;
              cr->move_to(center_x, center_y);
              cr->arc(center_x, center_y, radius, start_angle,
                      start_angle + angle_step);
              cr->close_path();
              j = n[i];
              setColor(cr, j == BG_COLOR ? BG_COLOR : figure_color[best.n[j]]);
              cr->fill();
            }
            cr->restore();
            std::sort(n.begin(), n.end());
            std::string s;
            for (auto &a : n) {
              if (a != BG_COLOR) {
                s += std::to_string(a + 1);
              }
            }
            draw_text(cr, cx, cy, s);
          }
        }
      }
    }

    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->set_line_width(1.0);
    for (i = 1; i < N; i++) {
      cr->move_to(0, i * Q + .5);
      cr->line_to(width, i * Q + .5);
      cr->move_to(i * Q + .5, 0);
      cr->line_to(i * Q + .5, height);
    }
    cr->stroke();
  }

  void draw_text(const Cairo::RefPtr<Cairo::Context> &cr, int square_x,
                 int square_y, std::string text) {
    const int Q = DRAW_AREA_SQUARE;

    Pango::FontDescription font_desc;
    font_desc.set_family("Times New Roman");
    font_desc.set_weight(Pango::Weight::BOLD);
    int current_pixel_size = Q;
    const int min_pixel_size = 6;
    int text_width;
    int text_height;
    auto layout = Pango::Layout::create(m_drawing_area.get_pango_context());

    while (current_pixel_size > min_pixel_size) {
      font_desc.set_absolute_size(current_pixel_size * Pango::SCALE);
      layout->set_font_description(font_desc);
      layout->set_text(text);
      layout->get_pixel_size(text_width, text_height);
      if (text_width <= Q && text_height <= Q) {
        break;
      }
      current_pixel_size--;
    }

    double text_x = square_x + (Q - text_width) / 2.0;
    double text_y = square_y + (Q - text_height) / 2.0;
    cr->set_source_rgb(0, 0, 0);
    cr->move_to(text_x, text_y);
    layout->show_in_cairo_context(cr);
  }

  void setButtonTimerText() {
    m_buttonTimer.set_label((timer ? "stop" : "start") + std::string(" timer"));
  }

  void updateSaveButton(std::string s) {
    m_buttonSave.set_label(s);
    Glib::signal_timeout().connect(
        [this]() -> bool {
          m_buttonSave.set_label(SAVE_PNG);
          return false;
        },
        SAVE_TIMER_MILLISECONDS);
  }

  void addLog() {
    //  std::ofstream file(LOG_FILE, std::ios::out | std::ios::trunc); //w+
    std::ofstream file(LOG_FILE, std::ios::app);
    std::string m(5, '-');
    file << "\n" << m << " " << dateTimeString() << " " << m << "\n";
    VFigure v;
    for (auto &a : figures) {
      if (!a.empty()) {
        v.push_back(a);
      }
    }
    file << to_string(field) + to_string(v) << "\n";

    for (auto &a : m_out) {
      file << a;
    }
    file.close();
  }

  bool tick() {
    int i;
    if (timer) {
      if (DEBUG_MODE) {
        from_string(fixed_field[NF], field, figures);
        m_out[1] = bestString();
        m_drawing_area.queue_draw();
      } else {
        gets();
        if (saveText) {
          addLog();
          saveText = 0;
          m_buttonSaveText.set_label(SAVE_TEXT);
        }
      }
      for (i = 0; i < NT; i++) {
        m_text_view[i].get_buffer()->set_text(m_out[i]);
      }
    }
    return !DEBUG_MODE;
  }

  void addStatistics(VPIntInt &v, int i) {
    auto it =
        std::find_if(v.begin(), v.end(), [&i](auto x) { return x.first == i; });
    if (it == v.end()) {
      v.push_back({i, 1});
    } else {
      it->second++;
    }
  }

  void gets() {
    int i, j, f = 0;
    bool log = 0;
    std::string s1, fields, s2;
    VString vs;
    for (auto &a : m_out) {
      a = "";
    }
    auto s = get_screenshot_winapi();
    if (!s.empty()) {
      best.setInvalid();
      m_out[0] = s;
      return;
    }
    m_drawing_area.queue_draw();

    i = 0;
    for (auto &a : figures) {
      if (a.empty()) {
        s2 = "";
      } else {
        s1 = "";
        // recognize lines, squares
        bool all = std::all_of(std::begin(a), std::end(a),
                               [](auto x) { return x.size() == 1; });
        if (all) {
          s1 += a.size() == 1 ? "dot" : std::to_string(a.size()) /*+ "V"*/;
        }
        if (s1.empty()) {
          all = std::all_of(std::begin(a), std::end(a), [](auto x) {
            return std::all_of(std::begin(x), std::end(x),
                               [](auto x) { return x == 1; });
          });
          if (all) {
            s1 += a.size() == 1 ? std::to_string(a[0].size()) /*+ "H"*/
                                : std::format("square{}", a.size());
            if (a.size() < 1 || a.size() > 3) { // was square5
              best.setInvalid();
              m_out[0] = "unrecognized1";
              return;
            }
          }
        }
        if (s1.empty()) {
          auto c = code(a);
          try {
            s1 = MAP.at(c);
          } catch (const std::out_of_range &e) {
            best.setInvalid();
            m_out[0] = "unrecognized";
            return;
          }
        }
        s2 = s1;
        s1 += " ";
        s += s1;
        VFigure recent;
        j = 0;
        for (auto &a : figures) {
          if (!a.empty() && j != i) {
            recent.push_back(a);
          }
          j++;
        }

        auto v = possibleMoves(a, recent, field);
        std::sort(v.begin(), v.end());
        for (auto &e : v) {
          s += e.to_string();
        }
        s += "\n";
      }
      i++;
      vs.push_back(s2);
    }
    f = countFill(field);
    fields = to_string(field);
    int possible = countPossible(field);

    i = std::count_if(vs.begin(), vs.end(), [](auto e) { return !e.empty(); });

    if (i == 3) {
      s1 = join(vs);
      if (f == 0) {
        newGame();
        startFromEmptyField = 1;
      }
      if (s1 != m_prev && fields != m_prevfields) {
        m_prev = s1;
        m_prevfields = fields;
        log = 1;
        addStatistics(possibleStatistics, possible);
        addStatistics(fillStatistics, f);

        i = 0;
        std::string cd, mincode;
        for (auto &e : vs) {
          mincode = code(figures[i]);
          cd = to_string(figures[i]);

          auto it =
              std::find_if(m_figureStatistics.begin(), m_figureStatistics.end(),
                           [&mincode](auto x) { return x.mincode == mincode; });
          if (it == m_figureStatistics.end()) {
            m_figureStatistics.push_back(FigureStatistics(e, mincode, cd));
          } else {
            auto it1 = std::find_if(it->v.begin(), it->v.end(),
                                    [&cd](auto x) { return x.first == cd; });
            if (it1 == it->v.end()) {
              it->v.push_back({cd, 1});
            } else
              it1->second++;
          }
          i++;
        }
      }
    }

    int total = 0, max = 0, squares = 0;
    for (auto &e : m_figureStatistics) {
      e.count();
      total += e.total;
      squares += e.squares;
      if (e.total > max) {
        max = e.total;
      }
    }

    std::sort(m_figureStatistics.begin(), m_figureStatistics.end());

    for (const auto &e : m_figureStatistics)
      s += e.to_string(total, max);

    auto elapsed = std::chrono::steady_clock::now() - gameBegin;
    auto duration_sec =
        std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    std::chrono::seconds sec{duration_sec};

    vs = {std::format("figures {}", total),
          std::format("squares {}", toString(squares, ',')),
          // std::format("map {}/{}", m_figureStatistics.size(), 5 + 2 +
          // MAP.size()),
          fillString(f), possibleString(possible, 1),
          std::format("time {:%T}{}", sec, startFromEmptyField ? "" : "*")};
    s += join(vs);

    s += possibleStatString();
    s += fillStatString();
    m_out[0] = s;
    m_out[1] = bestString();
    i = best.isInvalid() ? possible : best.estimate / 10000;
    m_out[2] = possibleString(i, 2);

    if (!DEBUG_MODE && LOG && log) {
      addLog();
      // savePng();
    }
  }

public:
  void on_show() override {
    Gtk::Window::on_show();

    // make_always_on_top
#ifdef _WIN32
    HWND hwnd = FindWindowA(NULL, m_title.c_str());
    if (hwnd != NULL) {
      SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
#endif
  }
  Gtk::ScrolledWindow m_scrolled_window[NT];
  Gtk::TextView m_text_view[NT];
  std::string m_out[NT];
  Gtk::Button m_buttonSave, m_buttonSaveText, m_buttonTimer;
  Gtk::DrawingArea m_drawing_area;
  std::string m_prev, m_prevfields;
  std::vector<FigureStatistics> m_figureStatistics;
  std::string m_title;
};

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create("com.example.myapp"
                                      // ,  Gio::Application::Flags::NON_UNIQUE
  );
  app->signal_startup().connect([app]() {
    auto display = Gdk::Display::get_default();
    if (display) {
      auto icon_theme = Gtk::IconTheme::get_for_display(display);
      icon_theme->add_resource_path("/com/example/myapp");
    }
  });
  return app->make_window_and_run<MyWindow>(argc, argv);
}

PointInfo getBase(int sx, int width, int sy, int height, int o) {
  int i, j, x, y, k;
  for (y = 0; y < height; y++) {
    auto p = gp + (y + sy) * gtotalWidth + sx;
    for (x = 0; x < width; x++, p++) {
      if (std::ranges::contains(FC[o], *p)) {
        return {x + sx, y + sy, *p};
      }
    }
  }
  return {-1, -1, 0};
}

uint32_t getPixelColor(int x, int y) { return gp[x + y * gtotalWidth]; }

std::string toABGR(uint32_t c, bool onlyRGB) {
  std::string s;
  int i, j;
  for (i = onlyRGB; i < 4; i++) {
    j = (c >> (8 * (3 - i))) & 0xff;
    s += (i == onlyRGB ? "" : ",") + std::to_string(j);
  }
  return s;
}

int colorDifference(uint32_t c1, uint32_t c2) {
  int i, j, r = 0;
  for (i = 0; i < 3; i++, c1 >>= 8, c2 >>= 8) {
    j = (c1 & 0xff) - (c2 & 0xff);
    r += std::abs(j);
  }
  return r;
}

void copy(const int source[N][N], int dest[N][N]) {
  for (int i = 0; i < N; ++i)
    std::copy(source[i], source[i] + N, dest[i]);
}

// o=1 for debug outputs
std::string dateTimeString(int o) {
  auto now = std::chrono::system_clock::now();
  std::time_t time_now = std::chrono::system_clock::to_time_t(now);
  std::tm *local_tm = std::localtime(&time_now);

  std::stringstream ss;
  ss << std::put_time(local_tm, o == 0 ? "%Y%m%d-%H%M%S" : "%H:%M:%S ");
  return ss.str();
}

std::string savePng() {
  auto [crop_x, crop_y, crop_w, crop_h] = picture_rectangle;
  // auto start = std::chrono::steady_clock::now();
  std::string s;
  int rowstride = gtotalWidth * 4;
  uint8_t *crop_start_ptr =
      reinterpret_cast<uint8_t *>(gp) + (crop_y * rowstride) + (crop_x * 4);

  auto pixbuf = Gdk::Pixbuf::create_from_data(
      crop_start_ptr,       // Указатель на первый пиксель кропа
      Gdk::Colorspace::RGB, // Цветовое пространство
      true,                 // Наличие альфа-канала (has_alpha)
      8,                    // Глубина цвета (bits_per_sample)
      crop_w,               // Ширина кропа
      crop_h,               // Высота кропа
      rowstride             // Шаг строки исходного (!) буфера
  );

  if (pixbuf) {
    try {
      s = dateTimeString() + ".png";
      pixbuf->save("./" + SCREEN_DIR + '/' + s, "png",
                   {"compression"}, // Вектор имен опций
                   {"9"} // Вектор значений опций (максимальное сжатие)
      );
      // auto end = std::chrono::steady_clock::now();
      // auto elapsed =
      //     std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
      // std::cout << std::format("time {}ms\n", elapsed.count());

    } catch (const Glib::Error &ex) {
      s = "error save PNG: " + std::string(ex.what());
      // std::cout << s << "\n";
    }
  } else {
    s = "cann't create pixbuf";
    // std::cout << s << "\n";
  }
  return s;
}

std::string get_screenshot_winapi() {
  // 1. Получаем контекст устройства всего экрана
  HDC hScreenDC = GetDC(NULL);
  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);

  // 2. Создаем битмап в памяти Windows
  HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

  // 3. Копируем экран в память
  BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);

  // 4. Подготавливаем структуру для извлечения пикселей в формате RGBA/BGRA
  BITMAPINFOHEADER bi;
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = width;
  bi.biHeight =
      -height; // Отрицательное значение, чтобы картинка не была перевернута
  bi.biPlanes = 1;
  bi.biBitCount = 32; // 4 байта на пиксель (BGRA)
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;

  gbuffer.resize(width * height * 4);
  GetDIBits(hMemoryDC, hBitmap, 0, height, gbuffer.data(), (BITMAPINFO *)&bi,
            DIB_RGB_COLORS);

  // Освобождаем ресурсы WinAPI
  SelectObject(hMemoryDC, hOldBitmap);
  DeleteObject(hBitmap);
  DeleteDC(hMemoryDC);
  ReleaseDC(NULL, hScreenDC);

  // 5. Конвертируем BGRA в RGBA (GTK ожидает RGBA канал)
  for (size_t i = 0; i < gbuffer.size(); i += 4) {
    uint8_t blue = gbuffer[i];
    gbuffer[i] = gbuffer[i + 2]; // Red
    gbuffer[i + 2] = blue;       // Blue
                                 // gbuffer[i+3] остается Alpha
  }

  gp = reinterpret_cast<uint32_t *>(gbuffer.data());
  gtotalWidth = width;

  PointInfo pa;
  int x, y, i, j, k, l, b, c;
  pa = getBase(0, width, 0, height, 0);

  x = pa.x;
  y = pa.y;
  if (x == -1)
    return "not found";
  x += DX;
  y += DY;

  if (x + STEP * (N - 1) >= width || y + STEP * (N - 1) >= height)
    return std::format("bounds error {}", __LINE__);

  l = 0;
  for (j = 0; j < N; j++) {
    for (i = 0; i < N; i++) {
      uint32_t k = getPixelColor(x + i * STEP, y + j * STEP);
      field[j][i] = b = k != EMPTY[j];
      if (b) {
        if (std::find(std::begin(POSSIBLE_COLOR), std::end(POSSIBLE_COLOR),
                      k) == std::end(POSSIBLE_COLOR)) {
          l++;
        }
      }
    }
  }
  if (l > 7) {
    return "bad field";
  }

  x += DX1;
  y += DY1;
  b = x;
  c = y;
  if (b + 2 * SX + SMALL_SQUARE_SIZE >= width ||
      c + SMALL_SQUARE_SIZE >= height)
    return std::format("bounds error {}", __LINE__);

  i = pa.x - 120;
  j = pa.y - 30;
  if (i < 0 || y < 0) {
    picture_rectangle = {0, 0, 1, 1};
    return std::format("bounds error {}", __LINE__);
  }
  picture_rectangle = {i, j, 440, 730};

  l = 0;
  for (auto &a : figures) {
    a.clear();
    pa = getBase(b, SMALL_SQUARE_SIZE, c, SMALL_SQUARE_SIZE, 1);
    if (pa.x != -1) {
      pa.y += 9;
      pa.color = getPixelColor(pa.x, pa.y);
      figure_color[l++] = pa.color; // only valid

      y = pa.y;
      for (j = 0; j < 5; j++, y += STEPS) {
        VInt q;
        x = b + (pa.x - b) % STEPS;
        for (i = 0; i < 5; i++, x += STEPS) {
          k = colorDifference(pa.color, getPixelColor(x, y));
          q.push_back(k < 77 ? 1 : 0);
        }

        if (std::ranges::find(q, 1) != q.end())
          a.push_back(q);
        else
          break;
      }

      x = N, y = -1;
      for (auto &numbers : a) {
        auto first_it = std::find(numbers.begin(), numbers.end(), 1);
        auto last_rit = std::find(numbers.rbegin(), numbers.rend(), 1);

        int first_index = std::distance(numbers.begin(), first_it);
        if (x > first_index) {
          x = first_index;
        }
        int last_index =
            (numbers.size() - 1) - std::distance(numbers.rbegin(), last_rit);
        if (y < last_index) {
          y = last_index;
        }
      }

      for (auto &numbers : a) {
        VInt sub_vector(numbers.begin() + x, numbers.begin() + y + 1);
        numbers = sub_vector;
      }
    }
    b += SX;
  }
  return "";
}

std::string to_string(const Figure &a, int o) {
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

std::string invert(const Figure &a, bool x, bool y) {
  return to_string(invertFigure(a, x, y));
}

std::string toString(int t, char separator, int digits) {
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

void make_move(int i, int j, const Figure &f, int field[N][N]) {
  int _x, _y, x, y, l;
  int fill[N][N], after[N][N];
  std::set<int> xa, ya;

  copy(field, fill);
  for (_y = 0; _y < f.size(); _y++) {
    for (_x = 0; _x < f[_y].size(); _x++) {
      if (f[_y][_x]) {
        printf("@");
        x = _x + i;
        y = _y + j;
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
}

void from_string(const std::string &s, int field[N][N], Figure figures[3]) {
  int i = 0, j = -1;
  for (auto &a : s) {
    j++;
    if (strchr("01", a)) {
      field[i / N][i % N] = a - '0';
      if (++i == N * N) {
        break;
      }
    }
  }

  Glib::ustring data = s.substr(j + 1);
  Glib::ustring s1 = R"(([^-\n]+))";
  Glib::ustring s2 = "\\s*-\\s*";
  Glib::ustring s3 = R"(\s+(\d{2})(?:\[\d+\])?_(\d))";
  auto regex =
      Glib::Regex::create(s1 + s2 + s1 + s2 + s1 + "(?:" + s3 + s3 + s3 + ")?");
  Glib::MatchInfo match_info;

  std::string g, g1;
  if (!regex->match(data, match_info)) {
    std::cout << "not match error line " << __LINE__;
    return;
  }
  j = match_info.get_match_count();
  for (i = 0; i < 3; i++) {
    g = match_info.fetch(i + 1);
    from_string(g, figures[i]);
  }
  const int moves = (j - 4) / 2;
  std::cout << std::format("mc{} {} \n", j, moves);
  for (i = 0; i < moves; i++) {
    g = match_info.fetch(4 + 2 * i);  // index from 0 so -'0'
    g1 = match_info.fetch(5 + 2 * i); // g1[0] - '1' because index starts from 1
    make_move(g[0] - '0', g[1] - '0', figures[g1[0] - '1'], field);
    // break;
  }
  // std::cout << std::format("{} \n", to_string(field));

  if (moves) { // just view moves
    for (i = 0; i < 3; i++) {
      figures[i].clear();
    }
  }
}

#ifdef MASK
bool subFigure(const Figure &inner, const Figure &outer) {
  if (inner.size() > outer.size() || inner[0].size() > outer[0].size()) {
    return false;
  }

  int x, y;
  for (y = 0; y < inner.size(); y++) {
    for (x = 0; x < inner[y].size(); x++) {
      if (!outer[y][x] && inner[y][x]) {
        return false;
      }
    }
  }

  return true;
}
#endif

std::string code(const Figure &a) {
  int x, y, i;
  auto r = rotate(a);
  std::string c, min = "2";
  for (x = 0; x < 2; x++) {
    for (y = 0; y < 2; y++) {
      for (i = 0; i < 2; i++) {
        c = invert(i ? r : a, x, y);
        if (c < min) {
          min = c;
        }
      }
    }
  }
  return min;
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
  int i, j;
  for (j = 0; j <= N - w; j++) {
    for (i = 0; i <= N - h; i++) {
      if (possibleRectange(i, j, field, w, h)) {
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

std::string possibleString(int i, const int o) {
  auto d = (1 - std::pow(1 - double(i) / ALL_COUNT, 3)) * 100;
  if (o == 0)
    return std::format(" {} {:.0f}%", i, d);
  else if (o == 1)
    return std::format("possible {} {:.2f}%", i, d);
  else
    return std::format("{} {:.1f}%", i, d);
}

std::string fillString(int i) {
  return std::format("fill {}/{}={:.2f}%", i, N * N, i * 100. / N / N);
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

std::string join(const VString &vs) {
  std::string s;
  for (auto &a : vs) {
    s += a + '\n';
  }
  return s;
}

std::string possibleStatString() {
  auto &v = possibleStatistics;
  std::string s = "possible statistics\n";
  std::sort(v.begin(), v.end(), [](auto &a, auto &b) {
    return a.second > b.second || a.second == b.second && a.first < b.first;
  });

  int sum = std::accumulate(v.begin(), v.end(), 0,
                            [](int acc, auto &e) { return acc + e.second; });

  double total = 0;
  for (auto &a : v) {
    s += std::format("{} {} {:.1f}%\n", a.first, a.second,
                     a.second * 100. / sum);

    auto d = std::pow(1 - double(a.first) / ALL_COUNT, 3);
    total += d * a.second / sum;
  }
  s += std::format("total({}) {} bad {:.1f}%\n", v.size(), sum, total * 100);
  return s;
}

std::string fillStatString() {
  auto &v = fillStatistics;
  std::string s = "fill statistics\n";
  std::sort(v.begin(), v.end(), [](auto &a, auto &b) {
    return a.second > b.second || a.second == b.second && a.first > b.first;
  });
  int sum = std::accumulate(v.begin(), v.end(), 0,
                            [](int acc, auto &e) { return acc + e.second; });

  for (auto &a : v) {
    s += std::format("{} {} {:.1f}%\n", a.first, a.second,
                     a.second * 100. / sum);
  }
  s += std::format("total({}) {}\n", v.size(), sum);
  return s;
}
