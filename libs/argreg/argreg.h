#ifndef ARGREG_H
#define ARGREG_H

#include <sstream>
#include <string>
#include <regex>
#include <vector>
#include <numeric>

namespace ar {

  struct argreg {
    public:
      argreg(int argc, char ** argv) {
        std::vector<std::string> args = {argv + 1, argv + argc};
        m_args = accumulate(
            args.begin(),
            args.end(),
            m_args, [](std::string a, std::string b) { return a + b + " "; });
      }

      template<typename ... alias>
        void sflag(unsigned long long conf_bit, alias &... aliases) {
          std::ostringstream oss;

          oss << "(";
          int tmp[] = {0, ((void)(oss << aliases << "|"), 0) ... };
          oss.seekp(-1, std::ios_base::end);
          oss << ")\\s{1}";

          std::regex expr(oss.str());
          std::smatch matches;
          std::regex_search(m_args, matches, expr);

          if (matches.size() > 0)
            m_conf |= 1ULL << conf_bit;
        }

      template<typename ... alias>
        bool gparam(std::string & value, alias &... aliases) {
          std::ostringstream oss;

          oss << "(";
          int tmp[] = {0, ((void)(oss << aliases << "|"), 0) ... };
          oss.seekp(-1, std::ios_base::end);
          oss << ")(\\s.+?(\\s|$)){1}";

          std::regex expr(oss.str());
          std::smatch matches;
          std::regex_search(m_args, matches, expr);

          if (matches.size() < 2) return false;

          std::string match = matches[2].str();

          match.replace(0, 1, "");
          match.replace(match.end(), match.end() - 1, "");

          value = match;

          return true;
        }

      bool operator [] (unsigned long long bit) {
        return m_conf & (1ULL << bit);
      }

    private:
      std::string m_args;

      unsigned long long m_conf = 0;
  };

}

#endif
