#include <filesystem>
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
#include <windows.h>
#include "common.h"

int timer = 1;
int saveText = 0;
const bool LOG = 1;
const int TIMER_MILLISECONDS = 500;
const int SAVE_TIMER_MILLISECONDS = 3000;
const double HIGHLIGHT_SUCCESS = .75;
const std::string LOG_FILE = "log.txt";
const std::string SCREEN_DIR = "png";

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
const std::vector<uint32_t> F_COLOR[] = {
    {0xffab2578}, {0xff59ed9e, 0xff45dcf7, 0xff7676ff, 0xffffb945, 0xfff65ae9}};
const uint32_t EMPTY_COLOR[] = {0xff9c2469, 0xff952463, 0xff8e245c, 0xff872355,
                                0xff7f224d, 0xff782247, 0xff702240, 0xff692139};
const uint32_t POSSIBLE_COLOR[] = {
    0xff59ed9e, 0xffffb945, 0xff45dcf7, 0xff45dcf7, 0xff7676ff, 0xfff65ae9,
    0xfff9a8f2, // violet+
    0xff7b978d, // violet@
    0xffb6a93a, // blue@
    0xff7fc63b, // blue@
    0xff93c89f, // blue@
    0xffed57e0, // violet@
};

uint32_t BG_COLOR = 0xE6D8AD;
// default color fo rebug mode
uint32_t figure_color[] = {0xff59ed9e, 0xffffb945, 0xff45dcf7};

const std::string SAVE_PNG = "save png";
const std::string SAVE_TEXT = "add text to log";
int gtotalWidth;
uint32_t *gp;
std::vector<uint8_t> gbuffer;
VPIntInt fillStatistics, possibleStatistics;
std::filesystem::path app_dir;

std::string fillString(int possible);
std::string dateTimeString(int o = 0);
std::string timeString() { return dateTimeString(1); }
std::string possibleStatString();
std::string fillStatString();
std::string savePng();
Glib::RefPtr<Gdk::Pixbuf> createPixbuf(int o);
std::string get_screenshot_winapi();
std::string toABGR(uint32_t c, bool onlyRGB = true);
std::string code(const Figure &a);
std::string fullPath(std::string name) { return (app_dir / name).string(); }

struct PointInfo {
  int x;
  int y;
  uint32_t color;
};

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

class Window : public Gtk::Window {
public:
  Window()
      : m_buttonSave(SAVE_PNG), m_buttonSaveText(SAVE_TEXT),
        m_buttonSearchPrize("search prize"), m_buttonTimer(),
        m_buttonClearLog("clear log") {
    int i;

    set_resizable(false);
    m_title = std::format("gtkmm {}.{}.{}", GTKMM_MAJOR_VERSION,
                          GTKMM_MINOR_VERSION, GTKMM_MICRO_VERSION);
    set_title(m_title);

    auto display = Gdk::Display::get_default();
    auto icon_theme = Gtk::IconTheme::get_for_display(display);
    icon_theme->add_search_path(app_dir.string());

    // set icon without file name. If file  "app.png", pass only "app"
    set_icon_name("app");

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
      m_text_view[i].set_wrap_mode(Gtk::WrapMode::WORD);
    }

    auto box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 2);
    box->append(m_buttonSave);
    box->append(m_buttonClearLog);
    box->append(m_buttonSaveText);
    box->append(m_buttonTimer);
    box->append(m_buttonSearchPrize);
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
    m_drawing_area.set_draw_func(sigc::mem_fun(*this, &Window::on_draw));

    m_buttonSave.signal_clicked().connect(
        [this]() { updateSaveButton(savePng()); });

    m_buttonSaveText.signal_clicked().connect([this]() {
      saveText = 1;
      m_buttonSaveText.set_label("waiting...");
    });

    m_buttonSearchPrize.signal_clicked().connect([this]() {
      if (timer) {
        timer = 0;
        setButtonTimerText();
      }
      int i = 1;
      m_out[i] = getPrizesString();
      m_text_view[i].get_buffer()->set_text(m_out[i]);
    });

    m_buttonTimer.signal_clicked().connect([this]() {
      timer = !timer;
      setButtonTimerText();
    });

    m_buttonClearLog.signal_clicked().connect([this]() { removeLog(); });

    setButtonTimerText();

    init();
    newGame();

    auto buffer = m_text_view[2].get_buffer();
    m_highlight_tag = buffer->create_tag("my_yellow_highlight");
    m_highlight_tag->property_background() = "yellow";
    m_highlight_tag->property_foreground() = "black";

    if (LOG)
      std::filesystem::create_directories("./" + SCREEN_DIR);

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Window::tick),
                                   TIMER_MILLISECONDS);
  }

  void highlightText(bool highlight) {
    auto buffer = m_text_view[2].get_buffer();
    Gtk::TextBuffer::iterator start = buffer->begin();
    Gtk::TextBuffer::iterator end = buffer->end();

    if (highlight) {
      buffer->apply_tag(m_highlight_tag, start, end);
    } else {
      buffer->remove_all_tags(start, end);
    }
  }

  void newGame() {
    gameBegin = std::chrono::steady_clock::now();
    best.setInvalid();
    fillStatistics.clear();
    possibleStatistics.clear();
    m_figureStatistics.clear();
  }

  void setColor(const Cairo::RefPtr<Cairo::Context> &cr, int i) {
    double b = ((i >> 16) & 0xFF) / 255.0;
    double g = ((i >> 8) & 0xFF) / 255.0;
    double r = (i & 0xFF) / 255.0;
    cr->set_source_rgb(r, g, b);
  }

  void draw_hatched_square(const Cairo::RefPtr<Cairo::Context> &cr, int i,
                           int j) {
    const int Q = DRAW_AREA_SQUARE;
    const int W = DRAW_AREA_SQUARE / 3;
    auto pattern_surface =
        Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, W, W);
    auto pattern_cr = Cairo::Context::create(pattern_surface);

    pattern_cr->set_source_rgb(0, 0, 0);
    pattern_cr->set_line_width(.5);
    pattern_cr->move_to(0, W);
    pattern_cr->line_to(W, 0);
    pattern_cr->stroke();

    auto pattern = Cairo::SurfacePattern::create(pattern_surface);
    pattern->set_extend(Cairo::Pattern::Extend::REPEAT);
    cr->set_source(pattern);
    cr->rectangle(i * Q, j * Q, Q, Q);
    cr->fill();
  }

  void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height) {
    int i, j, n, x, y, cx, cy;
    const int Q = DRAW_AREA_SQUARE;
    const bool usePixbuf = 0;
    VInt nf[N][N];

    // if (best.isInvalid()) {
    //   auto style_context = get_style_context();
    //   Gdk::RGBA bg_color;
    //   style_context->lookup_color("theme_bg_color", bg_color);
    //   Gdk::Cairo::set_source_rgba(cr, bg_color);
    //   cr->rectangle(0, 0, width, height);
    //   cr->fill();
    // }

    if (usePixbuf) {
      auto pixbuf = createPixbuf(1);
      int dest_width = Q * N;
      int dest_height = Q * N;
      Glib::RefPtr<Gdk::Pixbuf> resized_pixbuf =
          pixbuf->scale_simple(Q * N, Q * N, Gdk::InterpType::BILINEAR);

      Gdk::Cairo::set_source_pixbuf(cr, resized_pixbuf, 0, 0);
      cr->paint();
    } else {
      setColor(cr, BG_COLOR);
      for (y = 0; y < N; y++) {
        for (x = 0; x < N; x++) {
          if (field[y][x])
            cr->rectangle(x * Q, y * Q, Q, Q);
        }
      }
      cr->fill();
    }

    if (!best.isInvalid()) {
      VInt vi = getNonEmptyIndex();
      for (n = 0; n < vi.size(); n++) {
        i = best.gx(n);
        j = best.gy(n);
        if (i != InvalidValue) {
          for (auto &xy : ALL_FIGURES[vi[best.n[n]]].xy) {
            x = xy[0];
            y = xy[1];
            auto &v = nf[j + y][i + x];
            v.push_back(n);
            if (field[j + y][i + x] && !std::ranges::contains(v, BG_COLOR)) {
              v.push_back(BG_COLOR);
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

      if (!best.isPrizeInvalid()) {
        draw_hatched_square(cr, best.prize_x, best.prize_y);
      }
    }

    if (!usePixbuf) {
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
    std::ofstream file(fullPath(LOG_FILE), std::ios::app);
    std::string m(5, '-');
    file << "\n"
         << m << " " << dateTimeString() << " " << m << "\n"
         << to_string(field) + nonEmptyFiguresString() << "\n";

    for (auto &a : m_out) {
      file << a;
    }
    file.close();
  }

  void removeLog() { std::filesystem::remove(fullPath(LOG_FILE)); }

  bool tick() {
    int i;
    bool b;
    if (timer) {
      gets();
      if (!DEBUG_MODE) {
        if (saveText) {
          addLog();
          saveText = 0;
          m_buttonSaveText.set_label(SAVE_TEXT);
        }
      }
      for (i = 0; i < NT; i++) {
        m_text_view[i].get_buffer()->set_text(m_out[i]);
      }
      highlightText(successProbability(best.getPossibleAfter()) <
                    HIGHLIGHT_SUCCESS);
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
    std::string s1, fields, name, cd, mincode, s, se;
    for (auto &a : m_out) {
      a = "";
    }
    if (DEBUG_MODE) {
      parseInitialString();
    } else {
      s = get_screenshot_winapi();
      if (!s.empty()) {
        best.setInvalid();
        m_out[0] = s;
        return;
      }
    }
    m_drawing_area.queue_draw();

    for (auto &f : gfigureIndex) {
      if (f != ALL_COUNT) {
        auto &a = ALL_FIGURES[f];

        VInt recent;
        for (auto &n : gfigureIndex) {
          if (n != ALL_COUNT && n != f) {
            recent.push_back(n);
          }
        }

        auto v = possibleMoves(a, recent, field);
        std::sort(v.begin(), v.end());
        s += a.name + "\n";
        for (auto &e : v) {
          s += e.to_string() + "\n";
        }
      }
    }
    f = countFill(field);
    fields = to_string(field);
    int possible = countPossible(field);

    i = std::count_if(gfigureIndex.begin(), gfigureIndex.end(),
                      [](auto e) { return e != ALL_COUNT; });

    if (i == 3) {
      s1 = nonEmptyFiguresString();
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

        for (auto &n : gfigureIndex) {
          auto &fi = ALL_FIGURES[n];
          mincode = fi.mincode;
          cd = fi.code;
          name = fi.name;

          auto it =
              std::find_if(m_figureStatistics.begin(), m_figureStatistics.end(),
                           [&mincode](auto x) { return x.mincode == mincode; });
          if (it == m_figureStatistics.end()) {
            m_figureStatistics.push_back(FigureStatistics(name, mincode, cd));
          } else {
            auto it1 = std::find_if(it->v.begin(), it->v.end(),
                                    [&cd](auto x) { return x.first == cd; });
            if (it1 == it->v.end()) {
              it->v.push_back({cd, 1});
            } else
              it1->second++;
          }
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

    se = s;
    s = "";

    std::sort(m_figureStatistics.begin(), m_figureStatistics.end());

    for (const auto &e : m_figureStatistics)
      s += e.to_string(total, max);

    VString vs = {
        std::format("figures {}", total),
        std::format("squares {}", toString(squares, ',')),
        // fillString(f),
        // possibleString(possible, 1),
    };

    s += join(vs, '\n', 1);
    s += possibleStatString();
    s += fillStatString();
    s += se;
    m_out[0] = s;
    m_out[1] = bestString();
    m_out[2] = std::format(
        "{} now {}",
        best.isInvalid() ? "" : possibleString(best.getPossibleAfter(), 2),
        possibleString(possible, 2));

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
  Gtk::Button m_buttonSave, m_buttonSaveText, m_buttonTimer,
      m_buttonSearchPrize, m_buttonClearLog;
  Gtk::DrawingArea m_drawing_area;
  std::string m_prev, m_prevfields;
  std::vector<FigureStatistics> m_figureStatistics;
  std::string m_title;
  Glib::RefPtr<Gtk::TextTag> m_highlight_tag;
};


int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create("com.example.myapp"
                                      // ,  Gio::Application::Flags::NON_UNIQUE
  );
  app->signal_startup().connect([app, argv]() {
    app_dir = std::filesystem::absolute(argv[0]).parent_path();
    std::string css_path = fullPath("app.css");
    auto css_provider = Gtk::CssProvider::create();
    try {
      css_provider->load_from_path(css_path);

      auto display = Gdk::Display::get_default();

      Gtk::StyleContext::add_provider_for_display(
          display, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch (const Gtk::CssParserError &ex) {
      std::cerr << "cann't parse CSS: " << ex.what() << std::endl;
    } catch (const Glib::FileError &ex) {
      std::cerr << "error file reading: " << ex.what() << std::endl;
    }
  });
  return app->make_window_and_run<Window>(argc, argv);
}

PointInfo getBase(int sx, int width, int sy, int height, int o) {
  int i, j, x, y, k;
  for (y = 0; y < height; y++) {
    auto p = gp + (y + sy) * gtotalWidth + sx;
    for (x = 0; x < width; x++, p++) {
      if (std::ranges::contains(F_COLOR[o], *p)) {
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

// o=1 for debug outputs
std::string dateTimeString(int o) {
  auto now = std::chrono::system_clock::now();
  std::time_t time_now = std::chrono::system_clock::to_time_t(now);
  std::tm *local_tm = std::localtime(&time_now);

  std::stringstream ss;
  ss << std::put_time(local_tm, o == 0 ? "%Y%m%d-%H%M%S" : "%H:%M:%S ");
  return ss.str();
}

Glib::RefPtr<Gdk::Pixbuf> createPixbuf(int o) {
  auto [crop_x, crop_y, crop_w, crop_h] = picture_rectangle;
  if (o) {
    crop_x += 6;
    crop_y += 150;
    crop_h = crop_w = 428;
  }
  int rowstride = gtotalWidth * 4;
  uint8_t *crop_start_ptr =
      reinterpret_cast<uint8_t *>(gp) + (crop_y * rowstride) + (crop_x * 4);

  return Gdk::Pixbuf::create_from_data(crop_start_ptr, Gdk::Colorspace::RGB,
                                       true, 8, crop_w, crop_h, rowstride);
}

std::string savePng() {
  std::string s;
  auto pixbuf = createPixbuf(0);
  if (pixbuf) {
    try {
      s = dateTimeString() + ".png";
      pixbuf->save("./" + SCREEN_DIR + '/' + s, "png", {"compression"}, {"9"});

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
  HDC hScreenDC = GetDC(NULL);
  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);

  HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

  BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);

  BITMAPINFOHEADER bi;
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = width;
  bi.biHeight = -height;
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;

  gbuffer.resize(width * height * 4);
  GetDIBits(hMemoryDC, hBitmap, 0, height, gbuffer.data(), (BITMAPINFO *)&bi,
            DIB_RGB_COLORS);

  SelectObject(hMemoryDC, hOldBitmap);
  DeleteObject(hBitmap);
  DeleteDC(hMemoryDC);
  ReleaseDC(NULL, hScreenDC);

  for (size_t i = 0; i < gbuffer.size(); i += 4) {
    uint8_t blue = gbuffer[i];
    gbuffer[i] = gbuffer[i + 2]; // Red
    gbuffer[i + 2] = blue;       // Blue
                                 // gbuffer[i+3] Alpha
  }

  gp = reinterpret_cast<uint32_t *>(gbuffer.data());
  gtotalWidth = width;

  PointInfo pa;
  int x, y, i, j, k, l, n, b, c;
  std::string s;
  pa = getBase(0, width, 0, height, 0);

  x = pa.x;
  y = pa.y;
  if (x == -1)
    return "not found";
  x += DX;
  y += DY;

  if (x + STEP * (N - 1) >= width || y + STEP * (N - 1) >= height)
    return std::format("bounds error {}", __LINE__);

  hs = "";
  l = 0;
  for (j = 0; j < N; j++) {
    for (i = 0; i < N; i++) {
      uint32_t k = getPixelColor(x + i * STEP, y + j * STEP);
      field[j][i] = b = k != EMPTY_COLOR[j];
      if (b && !std::ranges::contains(POSSIBLE_COLOR, k)) {
        hs += std::format("{}{} 0x{:x}\n", i, j, k);
        l++;
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
  for (n = 0; n < 3; n++, b += SX) {
    pa = getBase(b, SMALL_SQUARE_SIZE, c, SMALL_SQUARE_SIZE, 1);
    if (pa.x == -1) {
      gfigureIndex[n] = ALL_COUNT;
      continue;
    }
    pa.y += 9;
    pa.color = getPixelColor(pa.x, pa.y);
    figure_color[l++] = pa.color; // only valid

    VVInt a;
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

    s = to_string(a);
    gfigureIndex[n] = findFigureIndex(s);
  }
  return "";
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
