#include <algorithm>
#include <chrono>
#include <format>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>

#ifdef GTK_MAJOR_VERSION
#else
#include <regex>
#include <unordered_map>
#include <cstring>
#endif

using VInt = std::vector<int>;
using Figure = std::vector<VInt>;

const int NF = 0;
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

};

const int N = 8;
int field[N][N];
Figure figures[3];

static_assert(NF >= -1 && NF < int(std::size(fixed_field)));
const std::unordered_map<std::string, std::string> MAP = {
    {"01 11 10", "z"}, {"01 11 01", "t"}, {"001 111", "l"}, {"101 111", "π"}, {"01 11", "corner"}, {"001 001 111", "CORNER"}};

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

void from_string(const std::string &s, int field[N][N], Figure figures[3])
{
    int i = 0, j = -1;
    for (auto &a : s)
    {
        j++;
        if (strchr("01", a))
        {
            field[i / N][i % N] = a - '0';
            if (++i == N * N)
            {
                break;
            }
        }
    }
    std::string data = s.substr(j + 1);
    std::string ss = R"(\s*)";
    std::string s1 = R"(([^-\n]+))";
    std::string s2 = "\\s*-\\s*";
    std::string s3 = R"((?:\s+(\d{2})(?:\[\d+\])?_(\d))?)";
    std::string g = ss+s1 + s2 + s1 + s2 + s1 + s3 + s3 + s3, g1;

#ifdef GTK_MAJOR_VERSION
    auto regex =
        Glib::Regex::create(g);
    Glib::MatchInfo match_info;

    if (!regex->match(data, match_info))
    {
        std::cout << "not match error line " << __LINE__;
        return;
    }
    j = match_info.get_match_count();
    for (i = 0; i < 3; i++)
    {
        g = match_info.fetch(i + 1);
        from_string(g, figures[i]);
    }
    const int moves = (j - 4) / 2;
    std::cout << std::format("mc{} {} \n", j, moves);
    for (i = 0; i < moves; i++)
    {
        g = match_info.fetch(4 + 2 * i);  // index from 0 so -'0'
        g1 = match_info.fetch(5 + 2 * i); // g1[0] - '1' because index starts from 1
        make_move(g[0] - '0', g[1] - '0', figures[g1[0] - '1'], field);
        // break;
    }
    // std::cout << std::format("{} \n", to_string(field));

    if (moves)
    { // just view moves
        for (i = 0; i < 3; i++)
        {
            figures[i].clear();
        }
    }
#else

    std::regex pattern(s+s1);
    std::smatch matches;

    if (!std::regex_match(data, matches, pattern))
    {
        std::cout << std::format("error {} {}\n",data, __LINE__);
        return;
    }

    for (i = 0; i < 3; i++)
    {
        g = matches[i + 1];
        from_string(g, figures[i]);
    }

    size_t total_groups = matches.size();
    // matches[0] — это всегда вся совпавшая строка целиком
    std::cout << "Полная строка: " << matches.size() << std::endl;

    // matches[1], [2], [3] — это группы в круглых скобках
    std::cout << "Год:  " << matches[1] << std::endl;
    std::cout << "Месяц: " << matches[2] << std::endl;
    std::cout << "День:  " << matches[3] << std::endl;

#endif
    // i = countPossible(field);
    // std::cout << i << "#\n";
    // exit(1);
}

std::string to_string(const int field[N][N])
{
    std::string s;
    int i, j;
    for (j = 0; j < N; j++)
    {
        for (i = 0; i < N; i++)
        {
            s += std::to_string(field[j][i]);
        }
        s += "\n";
    }
    return s;
}
