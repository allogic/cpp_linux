#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>
#include <ctime>

#include "../../libs/trender/trender.h"

#define DT 5
#define PI 3.141592

typedef float mat2x2[2][2];

struct vec2 {
  public:
    vec2(float x, float y) {
      vec[0] = x;
      vec[1] = y;
    }

    float & operator [] (unsigned int i) { return vec[i]; }

    vec2 & operator += (vec2 & v) {
      vec[0] += v[0];
      vec[1] += v[1];

      return *this;
    }

  private:
    float vec[2];
};

struct triangle {
  public:
    vec2 p, a, b, c;
};

vec2 operator * (vec2 & v, float f) {
  return {
    v[0] * f,
      v[1] * f
  };
}

vec2 operator * (vec2 & v, mat2x2 & m) {
  return {
    v[0] * m[0][0] + v[1] * m[0][1],
    v[0] * m[1][0] + v[1] * m[1][1]
  };
}

inline float randf(float min, float max) {
  return min + ((static_cast<float>(std::rand()) / RAND_MAX) * (min - max));
}

inline float remapf(float v, float f0, float f1, float t0, float t1) {
  return ((v - f0) / (f1 - f0)) * (t1 - t0) + t0;
}

inline vec2 remapv(vec2 & v, int w, int h) {
  return {
    remapf(v[0], -(w / 2), w / 2, 0, w),
      remapf(v[1], -(h / 2), h / 2, 0, h)
  };
}

inline float edgefu(vec2 & a, vec2 & b, vec2 & c) {
  return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

inline void translatexy(vec2 & v, vec2 d, float t) {
  v[0] += d[0] * t;
  v[1] += d[1] * t;
}

inline vec2 rotatez(vec2 & v, float a) {
  float s = std::sin(a);
  float c = std::cos(a);

  mat2x2 m = { c, -s, s, c };

  return v * m;
}

int main() {
  std::srand(std::time(NULL));

  std::cout << trd::utility::cs() << trd::utility::hc();

  trd::renderer<char> re = {
    0, 0, 94, 47
  };

  triangle t = {
    { 0.f, 0.f },
    { 0.f, 10.f },
    { 10.f, -10.f },
    { -10.f, -10.f }
  };

  vec2 dir = {
    1.f, 1.f
  };

  float ang = 0.f;

  for (;;) {
    ang += 0.02f;

    if (ang > 360.f)
      ang = 0.f;

    if      (t.p[0] < -40.f) dir[0] = 1.f;
    else if (t.p[1] < -25.f) dir[1] = 1.f;
    else if (t.p[0] > 40.f) dir[0] = -1.f;
    else if (t.p[1] > 25.f) dir[1] = -1.f;

    translatexy(t.p, dir, 0.5f);

    vec2 tpr = remapv(t.p, re.w(), re.h()); 

    vec2 a = rotatez(t.a, ang);
    vec2 b = rotatez(t.b, ang);
    vec2 c = rotatez(t.c, ang);

    a += tpr;
    b += tpr;
    c += tpr;

    for (unsigned int i = 0; i < re.w(); i++)
      for (unsigned int j = 0; j < re.h(); j++) {
        vec2 p = {
          static_cast<float>(i),
          static_cast<float>(j)
        };

        float w0 = edgefu(b, c, p);
        float w1 = edgefu(c, a, p);
        float w2 = edgefu(a, b, p);

        const unsigned int idx = i + j * re.w();

        if (w0 >= 0 && w1 >= 0 && w2 >= 0) re[idx].sym = '#';
        else                               re[idx].sym = ' ';
      }

    re(std::cout);

    std::this_thread::sleep_for(std::chrono::milliseconds(DT));
  }

  return 0;
}
