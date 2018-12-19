#include <iostream>

#include "framebuffer.h"

int main() {
  fb::framebuffer buf = {128, 128, 512, 512};

  while (true) {
    uint32_t rgba = rand() % 0xffffffff;

    for (uint32_t x = buf.x; x < buf.w; x++)
      for (uint32_t y = buf.y; y < buf.h; y++) {
        const uint32_t i = buf.index(x, y);

        buf[i    ] = rgba >> 0x18;
        buf[i + 1] = rgba >> 0x10;
        buf[i + 2] = rgba >> 0x08;
      }

    usleep(1000000);
  }

  return 0;
}
