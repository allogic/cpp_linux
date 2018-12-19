#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>

#include "../../libs/trender/trender.h"

#define DT 5

struct shift {
  public: 
    bool axis;
    bool inverted;
    unsigned int rc;
    unsigned int amount;
};

template<typename T>
struct table {
  public:
    table(unsigned int rows, unsigned int cols) :
      m_rows(rows),
      m_cols(cols),
      m_size(rows * cols),
      m_buffer(new T[rows * cols]) {}

    table(
        unsigned int rows,
        unsigned int cols,
        char * data) : table(rows, cols) {
      for (unsigned int i = 0; i < m_size; i++)
        m_buffer[i] = data[i];
    }

    virtual ~table() {
      delete[] m_buffer;
    }

    T & operator [] (unsigned int i) {
      return m_buffer[i];
    }

    const unsigned int & rows() const { return m_rows; }
    const unsigned int & cols() const { return m_cols; }
    const unsigned int & size() const { return m_size; }

  private:
    unsigned int m_rows, m_cols, m_size;

    T * m_buffer;
};

struct utility {
  public:
    static std::vector<shift> get_shifts(
        unsigned int num_shifts,
        unsigned int min_shifts,
        unsigned int max_shifts) {
      std::vector<shift> s;

      for (unsigned int i = 0; i < num_shifts; i++)
        s.push_back({
            std::rand() % 2,
            std::rand() % 2,
            static_cast<unsigned int>(std::rand() % 26),
            std::rand() % (max_shifts - min_shifts) + min_shifts
            });

      return s;
    }

    static table<char> get_vigenere_table() {
      table<char> t = {
        26,
        26
      };

      const char alphabet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

      unsigned int m = 0;

      for (unsigned int i = 0; i < 26; i++) {
        char n = 0;

        for (unsigned int j = 0; j < 26; j++)
          t[i + j * 26] = alphabet[(n++ + m) % 26];

        m++;
      }

      return t;
    }
};

template<typename T>
struct anim_shift_functor {
  public:
    void operator () (
        trd::renderer<T> & r,
        table<T> & t,
        unsigned int a,
        unsigned int d,
        unsigned int k,
        unsigned int n) const {
      for (unsigned int s = 0; s < n; s++) {
        if (a) shift_horizontal(t, k, d);
        else shift_vertical(t, k, d);

        for (unsigned int i = 0; i < t.rows(); i++)
          for (unsigned int j = 0; j < t.cols(); j++)
            r[i + j * t.rows()].sym = t[i + j * t.rows()];

        r(std::cout);

        std::this_thread::sleep_for(std::chrono::milliseconds(DT));
      }
    } 

  private:
    void shift_horizontal(
        table<T> & t,
        unsigned int k,
        unsigned int d) const {
      if (d) {
        T tmp = t[k];
        for (unsigned int j = 0; j < t.cols(); j++)
          t[k + j * t.rows()] = t[k + ((j + 1) % t.cols()) * t.rows()];
        t[k + (t.cols() - 1) * t.rows()] = tmp;
      } else {
        T tmp = t[k + (t.cols() - 1) * t.rows()];
        for (unsigned int j = t.cols() - 1; j > 0; j--)
          t[k + j * t.rows()] = t[k + ((j - 1) % t.cols()) * t.rows()];
        t[k] = tmp;
      }
    }

    void shift_vertical(
        table<T> & t,
        unsigned int k,
        unsigned int d) const {
      if (d) {
        T tmp = t[k * t.rows()];
        for (unsigned int i = 0; i < t.rows(); i++)
          t[i + k * t.rows()] = t[((i + 1) % t.rows()) + k * t.rows()];
        t[t.rows() - 1 + k * t.rows()] = tmp;
      } else {
        T tmp = t[t.rows() - 1 + k * t.rows()];
        for (unsigned int i = t.rows() - 1; i > 0; i--)
          t[i + k * t.rows()] = t[((i - 1) % t.rows()) + k * t.rows()];
        t[k * t.rows()] = tmp;
      }
    }
};

int main() {
  const unsigned int seed = 123;
  const unsigned int num_shifts = 256;
  const unsigned int min_shifts = 2;
  const unsigned int max_shifts = 64;
  const unsigned int speed = 5;

  std::cout << trd::utility::cs() << trd::utility::hc();

  trd::renderer<char> re = {
    8, 4, 26, 26
  };

  std::srand(seed);

  auto st = utility::get_shifts(num_shifts, min_shifts, max_shifts);
  auto vt = utility::get_vigenere_table();

  anim_shift_functor<char> asf;

  for (auto & s : st)
    asf(
        re,
        vt,
        s.axis,
        s.inverted,
        s.rc,
        s.amount
       );

  std::reverse(std::begin(st), std::end(st));

  for (auto & s : st)
    asf(
        re,
        vt,
        s.axis,
        !s.inverted,
        s.rc,
        s.amount
       );

  std::cout << trd::utility::sc() << std::endl;

  return 0;
}
