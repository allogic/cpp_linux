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
  struct rect {
    public:
      uint32_t x, y, w, h;
  };

  struct index {
    public:
      uint32_t x, y, c;

      enum c {
        R = 0, G = 1, B = 2, T = 3
      };
  };

  struct framebuffer {
    public:
      framebuffer() = default;
      framebuffer(const framebuffer &) = default;
      framebuffer(const rect r, const char * fbd = FB_DEVICE) : wrect(r), fbfd(open(fbd, O_RDWR)) {
        if (!fbfd) std::cerr<< "Cannot open framebuffer device" << std::endl;

        if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) std::cerr << "Cannot read fixed screen information" << std::endl;
        if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) std::cerr << "Cannot read variable screen information" << std::endl;

        ssize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

        fb = static_cast<uint8_t *>(mmap(0, ssize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0));
      }

      ~framebuffer() {
        munmap(fb, ssize);
        close(fbfd);
      }

      uint8_t & operator [] (index i) {
        uint32_t idx = (i.x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (i.y + vinfo.yoffset) * finfo.line_length;
        return *(fb + idx + i.c);
      }

    private:
      const rect wrect;

      uint32_t ssize;
      uint32_t isize;

      fb_var_screeninfo vinfo;
      fb_fix_screeninfo finfo;

      const int fbfd;

      uint8_t * fb = nullptr;
  };
}

#endif
