# Linux Framebuffer
A simple c++ wrapper for easy access to the linux framebuffer.

## Setup
Check if it is already working with ```cat /dev/urandom > /dev/fb0```, if you do not get random pixels on the monitor check your permissions on the device.

## Usage
Here is a simple example how you can use the wrapper.

```
#include <iostream>

#include "fb.h"

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
```
