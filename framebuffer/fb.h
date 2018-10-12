#ifndef FB_H
#define FB_H

#include <iostream>
#include <cstdint>

extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/fb.h>

#include <sys/mman.h>
#include <sys/ioctl.h>
}

#define FB_DEVICE "/dev/fb0"

namespace fb {
  struct framebuffer {
    public:
      const uint32_t x, y, w, h;

      framebuffer(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h, const char * fbd = FB_DEVICE) :
        x(x),
        y(y),
        w(w),
        h(h),
        m_fbfd(open(fbd, O_RDWR)) {
          if (!m_fbfd) std::cerr<< "Cannot open framebuffer device" << std::endl;

          if (ioctl(m_fbfd, FBIOGET_FSCREENINFO, &m_fix_info)) std::cerr << "Cannot read fixed screen information" << std::endl;
          if (ioctl(m_fbfd, FBIOGET_VSCREENINFO, &m_var_info)) std::cerr << "Cannot read variable screen information" << std::endl;

          m_screen_size = m_var_info.xres * m_var_info.yres * m_var_info.bits_per_pixel / 8;

          m_fb = static_cast<uint8_t *>(mmap(0, m_screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fbfd, 0));
        }

      ~framebuffer() {
        munmap(m_fb, m_screen_size);
        close(m_fbfd);
      }

      uint32_t index(uint32_t x, uint32_t y) {
        return (x + m_var_info.xoffset) * (m_var_info.bits_per_pixel / 8) + (y + m_var_info.yoffset) * m_fix_info.line_length;
      }

      uint8_t & operator [] (uint32_t i) {
        return *(m_fb + i);
      }

    private:
      uint32_t m_screen_size;

      fb_var_screeninfo m_var_info;
      fb_fix_screeninfo m_fix_info;

      const int32_t m_fbfd;

      uint8_t * m_fb = nullptr;
  };
}

#endif
