# Linux Framebuffer
A simple c++ wrapper for easy access to the linux framebuffer.

## Setup
We must be in the video group in order to access the framebuffer device.

Check if it is already working with ```cat /dev/urandom > /dev/fb0```.
If you do not get random pixels on the monitor continue with ```ls -l /dev/fb0```.

```crw-rw---- 1 root video 29, 0 Oct  9 09:56 /dev/fb0```

Add yourself to the ```video``` group with ```sudo usermod -a -G video $USER```.

You might have to restart your system at this point to source all privileges.

## Usage
Here is a simple example how you can use the wrapper.

```
#include <iostream>
#include <cstdint>

#include "fb.h"

int main() {
  const fb::rect r = {128, 128, 512, 512};

  fb::framebuffer b(r);

  while (true) {
    for (uint32_t i = r.x; i < r.w; i++)
      for (uint32_t j = r.y; j < r.h; j++) {
        b[{i, j, fb::index::R}] = rand() % UINT8_MAX;
        b[{i, j, fb::index::G}] = rand() % UINT8_MAX;
        b[{i, j, fb::index::B}] = rand() % UINT8_MAX;
      }
  }

  return 0;
}
```
