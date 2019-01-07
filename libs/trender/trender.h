#ifndef TRENDER_H
#define TRENDER_H

#define CSI "\033["

namespace trd {

  enum COLOR {
    FG_BLACK      = 30, FGB_BLACK     = 90,
    FG_RED        = 31, FGB_RED       = 91,
    FG_GREEN      = 32, FGB_GREEN     = 92,
    FG_YELLOW     = 33, FGB_YELLOW    = 93,
    FG_BLUE       = 34, FGB_BLUE      = 94,
    FG_MAGENTA    = 35, FGB_MAGENTA   = 95,
    FG_CYAN       = 36, FGB_CYAN      = 96,
    FG_WHITE      = 37, FGB_WHITE     = 97,

    BG_BLACK      = 40, BGB_BLACK     = 100,
    BG_RED        = 41, BGB_RED       = 101,
    BG_GREEN      = 42, BGB_GREEN     = 102,
    BG_YELLOW     = 43, BGB_YELLOW    = 103,
    BG_BLUE       = 44, BGB_BLUE      = 104,
    BG_MAGENTA    = 45, BGB_MAGENTA   = 105,
    BG_CYAN       = 46, BGB_CYAN      = 106,
    BG_WHITE      = 47, BGB_WHITE     = 107,
  };

  struct utility {
    public:
      static inline const char * cs() { return CSI "2J"; }
      static inline const char * hc() { return CSI "?25l"; }
      static inline const char * sc() { return CSI "?25h"; }
  };

  struct graphic_mode {
    public:
      COLOR fg = COLOR::FGB_WHITE;
      COLOR bg = COLOR::BG_BLACK;
  };

  template<typename T>
    struct glyph {
      public:
        T sym;

        graphic_mode gm;
    };

  template<typename T>
    struct renderer {
      public:
        renderer(
            unsigned int x,
            unsigned int y,
            unsigned int w,
            unsigned int h) :
          m_x(x),
          m_y(y),
          m_w(w),
          m_h(h),
          m_s(w * h),
          m_glyphs(new glyph<T>[w * h]) {}

        virtual ~renderer() {
          delete[] m_glyphs;
        }

        glyph<T> & operator [] (unsigned int idx) { return m_glyphs[idx]; }

        void operator () (std::ostream & os) {
          for (unsigned int i = 0; i < m_w; i++)
            for (unsigned int j = 0; j < m_h; j++) {
              const unsigned int idx = i + j * m_w;

              pos(os, m_x + i, m_y + j);
              sgm(os, m_glyphs[idx].gm);

              os << m_glyphs[idx].sym;
            }
        }

        inline unsigned int w() const { return m_w; }
        inline unsigned int h() const { return m_h; }

      private:
        unsigned int m_x, m_y, m_w, m_h, m_s;

        glyph<T> * m_glyphs;

        inline void pos(
            std::ostream & os,
            unsigned int i,
            unsigned int j) {
          os << CSI << (j + 1) << ';' << (i + 1) << 'H';
        }

        inline void sgm(
            std::ostream & os,
            graphic_mode & gm) {
          os << CSI << gm.fg << ';' << gm.bg << 'm';
        }
    };

}

#endif
