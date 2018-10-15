# Argument Parser
A simple regex argument parser for easy command line arguments. Supports 64 commands and aliases.

## Setup
Include the header and start configuring.

## Usage
Here is a simple example how you can use the parser.

```
./main -h -v -b64e -gz -i=./inp.txt -o=./out.txt
```

```
#include <iostream>

#include "arg.h"

int main(int argc, char ** argv) {
  ap::argparser argp(argc, argv);

  argp.conf(std::regex("(--help|-h)"), 0ULL);
  argp.conf(std::regex("(--version|-v)"), 1ULL);

  argp.conf(std::regex("(--base64encode|-b64e)"), 16ULL);
  argp.conf(std::regex("(--base64decode|-b64d)"), 17ULL);

  argp.conf(std::regex("(--gzip|-gz)"), 63ULL);

  if (argp[0ULL]) std::cout << "Print help page" << std::endl;
  if (argp[1ULL]) std::cout << "Print version page" << std::endl;

  if (argp[16ULL] && argp[63ULL]) std::cout << "Base 64 encode and gzip file" << std::endl;

  std::string inp_file, out_file;

  if (!argp.get(std::regex("(--inp|-i)=(.+?)$"), inp_file))
    std::cout << "Missing input file path" << std::endl;

  if (!argp.get(std::regex("(--out|-o)=(.+?)$"), out_file))
    std::cout << "Missing output file path" << std::endl;

  if (!inp_file.empty() && !out_file.empty())
    std::cout << inp_file << std::endl << out_file << std::endl;

  return 0;
}
```
