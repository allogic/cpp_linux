# Linux Kernel Modesetting
An advanced c++ wrapper for easy kernel mode setting in linux.
Create or debug algorithms with a full RGBA buffer in realtime.

## Setup
Make sure ```libdrm-dev``` is installed on your system.

## Usage
Execute the program without a display-server.

```cpp
#include <iostream>
#include <vector>
#include <cmath>

#include "processing.h"

int32_t main() {
  int32_t fd;

  if (proc::system::open_fd(fd)) {
    std::vector<proc::device> devices;

    if (proc::hardware::open_devices(fd, devices)) {
      if (proc::hardware::init_devices(fd, devices)) {

        proc::event::on_draw = [&](proc::buffer & buf) {
          uint32_t i, j, k;

          uint8_t r, g, b;

          for (j = 0; j < buf.height; j++)
            for (i = 0; i < buf.width; i++) {
              k = buf.stride * j + i * 4;

              r = std::rand() % 0xff;
              g = std::rand() % 0xff;
              b = std::rand() % 0xff;

              buf[k] = (r << 0x10) | (g << 0x8) | b;
            }
        };

        proc::event::on_close = [&](int32_t sig) {
          proc::hardware::close_devices(fd, devices);
          proc::system::close_fd(fd);
        };

        proc::event::dispatch(fd, devices[0]);
      }

      proc::hardware::close_devices(fd, devices);
    }

    proc::system::close_fd(fd);
  }

  return 0;
}
```
