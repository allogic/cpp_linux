#ifndef ARG_H
#define ARG_H

#include <string>
#include <regex>

namespace ap {
  struct argparser {
    public:
      argparser(int argc, char ** argv) : m_argc(argc), m_argv(argv) {}

      void conf(std::regex expr, unsigned long long config_bit) {
        for (int i = 1; i < m_argc; i++)
          if (std::regex_match(m_argv[i], expr))
            m_config |= 1UL << config_bit;
      }

      bool get(std::regex expr, std::string & value) {
        std::smatch matches;

        for (int i = 1; i < m_argc; i++) {
          std::string s = m_argv[i];
          if (std::regex_search(s, matches, expr))
            if (matches.size() == 3) {
              value = matches[2].str();
              return 1;
            }
        }

        return 0;
      }

      bool operator [] (unsigned long long p) {
        return m_config & (1UL << p);
      }

    private:
      int m_argc = 0;
      char ** m_argv = nullptr;
      unsigned long long m_config = 0;
  };
}

#endif
