# Linux Terminal Renderer
A simple c++ terminal escape sequence renderer.

## Setup
Include the header and draw stuff in the terminal.

## Usage

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

#include "trender.h"

#define DT 5

int main() {
  std::cout << trd::utility::cs() << trd::utility::hc();

  trd::renderer<char> re = {
    8, 4, 32, 16
  };

  const char * abc = "abc";

  for (;;) {
    for (unsigned int i = 0; i < re.w(); i++)
      for (unsigned int j = 0; j < re.h(); j++) {
        const unsigned int off = i + j * re.w();

        re[off].sym = abc[std::rand() % 3];

        re[off].gm.fg = (std::rand() % 2) ?
          trd::COLOR::FG_GREEN :
          trd::COLOR::FGB_GREEN;
      }

    re(std::cout);

    std::this_thread::sleep_for(std::chrono::milliseconds(DT));
  }
}
```
