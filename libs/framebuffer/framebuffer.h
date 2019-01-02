#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <csignal>
#include <functional>

extern "C" {
#include <unistd.h>
#include <fcntl.h>

#include <linux/fb.h>

#include <sys/mman.h>
#include <sys/ioctl.h>
}

#define DEVICE "/dev/fb0"

namespace fb {

  struct buffer {
    public:
      fb_fix_screeninfo fb_fix_si;
      fb_var_screeninfo fb_var_si;

      uint32_t screen_size;

      uint32_t * map = nullptr;

      uint32_t w() const { return fb_var_si.xres; }
      uint32_t h() const { return fb_var_si.yres; }
      uint32_t s() const { return fb_var_si.xres * fb_var_si.yres; }

      uint32_t & operator [] (uint32_t i) { return *(map + i); }
  };

  static std::function<void(int32_t)> event_handler;

  struct event {
    public:
      static void on_close(std::function<void(int32_t)> ev) {
        event_handler = ev;

        std::signal(SIGINT, signal_handler);
      }

    private:
      static void signal_handler(int32_t sig) {
        event_handler(sig);

        std::_Exit(EXIT_SUCCESS);
      }
  };

  struct device {
    public:
      static bool open_fd(int32_t & fd, const char * device = DEVICE) {
        fd = open(device, O_RDWR);

        if (!fd) {
          std::cout << "cannot open file descriptor" << std::endl;

          return false;
        }

        return true;
      }

      static bool map_buffer(int32_t & fd, buffer & buf) {
        if (ioctl(fd, FBIOGET_FSCREENINFO, &buf.fb_fix_si)) {
          std::cout << "cannot read fixed screen information" << std::endl;

          return false;
        }

        if (ioctl(fd, FBIOGET_VSCREENINFO, &buf.fb_var_si)) {
          std::cout << "cannot read variable screen information" << std::endl;

          return false;
        }

        buf.screen_size = buf.s() * buf.fb_var_si.bits_per_pixel / 8;

        buf.map = static_cast<uint32_t *>(mmap(
              0,
              buf.screen_size,
              PROT_READ | PROT_WRITE,
              MAP_SHARED,
              fd,
              0));
      }

      static void unmap_buffer(buffer & buf) {
        munmap(buf.map, buf.screen_size);
      }

      static void close_fd(int32_t & fd) {
        close(fd);

        fd = 0;
      }
  };

}

#endif
