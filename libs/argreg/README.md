# Argument Parser
A simple regex argument parser for easy command line arguments.

## Setup
Include the header and start configuring.

## Usage

```cpp
#include <iostream>

#include "argreg.h"

#define CLEAR_BIT          0ULL
#define COLOR_BUFFER_BIT   1ULL

int main(int argc, char ** argv) {
  ar::argreg argr(argc, argv);

  argr.sflag(CLEAR_BIT, "--clear", "-c");
  argr.sflag(COLOR_BUFFER_BIT, "--color-buffer", "-cb");

  if (argr[CLEAR_BIT]) std::cout << "Clear bit set" << std::endl;
  if (argr[COLOR_BUFFER_BIT]) std::cout << "Color buffer bit set" << std::endl;

  std::string inp_path, out_path;

  if (!argr.gparam(inp_path, "--input", "-i"))
    std::cout << "Missing input file path" << std::endl;

  if (!argr.gparam(out_path, "--output", "-o"))
    std::cout << "Missing output file path" << std::endl;

  std::cout << inp_path << std::endl;
  std::cout << out_path << std::endl;

  return 0;
}
```
