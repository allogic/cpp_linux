#include <iostream>
#include <cstdint>

#include "framebuffer.h"

int main() {
  int32_t fd;

  if (fb::device::open_fd(fd)) {
    fb::buffer buf;

    if (fb::device::map_buffer(fd, buf)) {
      fb::event::on_close([&](int32_t sig) {
          fb::device::unmap_buffer(buf);
          fb::device::close_fd(fd);
          });

      for (;;)
        for (uint32_t i = 128; i < 512; i++)
          for (uint32_t j = 128; j < 512; j++)
            buf[i + j * 4 * buf.w() / 4] =
              (std::rand() % 0xff << 0x10) |
              (std::rand() % 0xff << 0x08) |
              (std::rand() % 0xff        );
    }
  }

  return 0;
}
