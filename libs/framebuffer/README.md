# Linux Framebuffer
A simple c++ wrapper for easy access to the linux framebuffer.
Create or debug algorithms with a full RGBA buffer. (not suitable for realtime)

## Setup
Check if it is already working with ```cat /dev/urandom > /dev/fb0```, if you do not get random pixels on the monitor check your permissions on the device or run it without a display-server.
If you wanna use it alongside X11, add or create ```/etc/xorg.conf``` with the following content:

```
Section "Device"
  Identifier  "Card0"
  Driver      "fbdev"
  Option      "fbdev" "/dev/fb0"
EndSection
```

## Usage

```cpp
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
```
