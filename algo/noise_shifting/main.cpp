#include <iostream>
#include <string>
#include <climits>
#include <chrono>
#include <thread>

#include "FastNoise.h"

#include "../../libs/trender/trender.h"

#define DT 5

const std::string shades =
"$@B%8&WM#"
"*oahkbdpq"
"wmZO0QLCJ"
"UYXzcvunx"
"rjft/\\|("
")1{}[]?-_"
"+~<>i!lI;"
":,\"^`'. ";

inline float remap(float v, float f0, float f1, float t0, float t1) {
  return ((v - f0) / (f1 - f0)) * (t1 - t0) + t0;
}

inline char shadetoasci(float c) {
  return shades[static_cast<int>(remap(c, -1.0f, 1.0f, 0.0f, 70.0f))];
}

int main() {
  std::cout << trd::utility::cs() << trd::utility::hc();

  trd::renderer<char> re = {
    0, 0, 128, 64
  };

  FastNoise fn;

  fn.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
  fn.SetFrequency(0.01f);

  fn.SetFractalType(FastNoise::FractalType::FBM);
  fn.SetFractalOctaves(6);
  fn.SetFractalLacunarity(2.0f);
  fn.SetFractalGain(0.5f);

  float x, y, z;

  x = y = z = std::rand() % (SHRT_MAX - SHRT_MIN) + SHRT_MAX;

  for (;;) {
    x = y = z += 0.2f;

    for (unsigned int i = 0; i < re.w(); i++)
      for (unsigned int j = 0; j < re.h(); j++) {
        const unsigned int off = i + j * re.w();

        float col = fn.GetNoise(x + i, y + j, z);

        re[off].gm.fg = (col > 0.0f) ?
          trd::COLOR::FGB_WHITE :
          trd::COLOR::FG_WHITE;
        re[off].sym = shadetoasci(col);
      }

    re(std::cout);

    std::this_thread::sleep_for(std::chrono::milliseconds(DT));
  }

  std::cout << trd::utility::sc() << std::endl;

  return 0;
}
