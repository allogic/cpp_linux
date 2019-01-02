#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <thread>

#include "../../libs/trender/trender.h"

#define MAX_STEPS 100
#define MAX_DIST 100.0f
#define SURF_DIST 0.01f

#define DELTA_TIME 5

const std::string shades =
"$@B%8&WM#"
"*oahkbdpq"
"wmZO0QLCJ"
"UYXzcvunx"
"rjft/\\|("
")1{}[]?-_"
"+~<>i!lI;"
":,\"^`'. ";

struct vec3 {
  public:
    float x, y, z;

    vec3 & operator /= (float v) {
      x /= v; y /= v; z /= v;

      return *this;
    }
};

vec3 operator + (const vec3 & a, float b) {
  return { a.x + b, a.y + b, a.z + b };
}

vec3 operator - (const vec3 & a, float b) {
  return { a.x - b, a.y - b, a.z - b };
}

vec3 operator * (const vec3 & a, float b) {
  return { a.x * b, a.y * b, a.z * b };
}

vec3 operator + (const vec3 & a, const vec3 & b) {
  return { a.x + b.x, a.y + b.y, a.z + b.z };
}

vec3 operator - (const vec3 & a, const vec3 & b) {
  return { a.x - b.x, a.y - b.y, a.z - b.z };
}

vec3 operator * (const vec3 & a, const vec3 & b) {
  return { a.x * b.x, a.y * b.y, a.z * b.z };
}

inline float mag(const vec3 & v) {
  return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline void norm(vec3 & v) {
  float m = mag(v);
  if (m > 0.0f) v /= m;
}

inline float dot(const vec3 & a, const vec3 & b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float clamp(float v, float min, float max) {
  if (v < min) return min;
  if (v > max) return max;

  return v;
}

inline float remap(float v, float f0, float f1, float t0, float t1) {
  return ((v - f0) / (f1 - f0)) * (t1 - t0) + t0;
}

float getd(const vec3 & pos) {
  vec3 sp = { 0.0f, 1.0f, 6.0f };

  float sd = mag(pos - sp) - 1.0f;
  float pd = pos.y;

  return std::min(sd, pd);
}

float raym(const vec3 & ori, const vec3 & dir) {
  float d;

  for (unsigned int i = 0; i < MAX_STEPS; i++) {
    float s = getd(ori + dir * d);

    d += s;

    if (d > MAX_DIST || s < SURF_DIST) break;
  }

  return d;
}

vec3 getn(const vec3 & pos) {
  float d = getd(pos);

  vec3 e0 = { 0.01f, 0.0f, 0.0f };
  vec3 e1 = { 0.0f, 0.01f, 0.0f };
  vec3 e2 = { 0.0f, 0.0f, 0.01f };

  float d0 = d - getd(pos - e0);
  float d1 = d - getd(pos - e1);
  float d2 = d - getd(pos - e2);

  vec3 n = { d0, d1, d2 };

  norm(n);

  return n;
}

float getl(const vec3 & pos, float delta) {
  vec3 lp = { 0.0f, 5.0f, 6.0f };

  lp.x += std::sin(delta) * 2.0f;
  lp.z += std::cos(delta) * 2.0f;

  vec3 l = lp - pos;

  norm(l);

  vec3 n = getn(pos);

  float d = clamp(dot(n, l), 0.0f, 1.0f);

  float sd = raym(pos + n * SURF_DIST, l);

  if (sd < mag(lp - pos)) d *= -0.1f;

  return d;
}

int main() {
  const unsigned int width = 128;
  const unsigned int height = 64;

  std::cout << trd::utility::cs() << trd::utility::hc();

  trd::renderer<char> re = {
    0, 0, width, height
  };

  float delta;

  for (;;) {
    delta += 0.05f;

    for (unsigned int i = 0; i < width; i++)
      for (unsigned int j = 0; j < height; j++) {
        float u = (i - 0.5f * width) / width;
        float v = (j - 0.5f * height) / height;

        vec3 cam = { 0.0f, 1.0f, 0.0f };
        vec3 dir = { u, v, 1.0f };

        norm(dir);

        float d = raym(cam, dir);
        float c = getl(cam + dir * d, delta);

        float o = remap(c, 0.0f, 1.0f, 0.0f, shades.size());

        unsigned int off = i + j * width;

        re[off].gm.fg = (o < 0.5f) ?
          trd::COLOR::FGB_WHITE :
          trd::COLOR::FG_WHITE;
        re[off].sym = shades[static_cast<int>(o)];
      }

    re(std::cout);

    std::this_thread::sleep_for(std::chrono::milliseconds(DELTA_TIME));
  }

  std::cout << trd::utility::sc() << std::endl;

  return 0;
}
