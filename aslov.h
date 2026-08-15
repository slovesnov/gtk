#include<vector>
#include<string>

using VString=std::vector<std::string>;

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

VString split(const std::string &subject, const std::string &separator) {
	VString r;
	size_t pos, prev;
	for (prev = 0; (pos = subject.find(separator, prev)) != std::string::npos;
			prev = pos + separator.length()) {
		r.push_back(subject.substr(prev, pos - prev));
	}
	r.push_back(subject.substr(prev, subject.length()));
	return r;
}

std::string ltrim(const std::string &s) {
	std::string::const_iterator it;
	for (it = s.begin(); it != s.end() && isspace(*it); it++)
		;
	return s.substr(it - s.begin());
}

std::string rtrim(const std::string &s) {
	std::string::const_reverse_iterator it;
	for (it = s.rbegin(); it != s.rend() && isspace(*it); it++)
		;
	return s.substr(0, s.length() - (it - s.rbegin()));
}

std::string trim(const std::string &s) {
	std::string q = ltrim(s);
	return rtrim(q);
}
