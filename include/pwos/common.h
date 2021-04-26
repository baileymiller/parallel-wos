#pragma once

#include <pwos/fwd.h>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <map>
#include <math.h>
#include <memory>
#include <omp.h>
#include <pcg32.h>
#include <random>
#include <string>
#include <vector>

using std::string;
using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::cout;
using std::endl;
using std::map;

// Macros / Constants
#define _USE_MATH_DEFINES

typedef Eigen::Vector<float, Eigen::Dynamic> Vectorf;
typedef Eigen::Vector<int, 2> Vec2i;
typedef Eigen::Vector<float, 2> Vec2f;
typedef Eigen::Vector<float, 3> Vec3f;
typedef Eigen::Vector<float, 4> Vec4f;

inline void THROW(bool cond, string message)
{
    throw std::runtime_error(message);
}

inline void THROW_IF(bool cond, string message)
{
    if (cond) throw std::runtime_error(message);
}

#define BOUNDARY_EPSILON 1e-2

//========================//
// Helper functions       //
//========================//
inline pcg32 getSampler(int id = 0)
{
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dis;
  uint64_t seed = dis(gen);
  return pcg32(seed + id);
}

inline Vec2f sampleCirclePoint(float R, float rand)
{
    float theta = rand * 2 * M_PI;
    return Vec2f(cos(theta), sin(theta)) * R;
}

Vec2f getXYCoords(Vec2i pixel, Vec4f window, Vec2f res)
{
    float dx = window[2] - window[0];
    float dy = window[3] - window[1];
    return Vec2f(
        pixel.x() / float(res.x()) * dx + window[0],
        pixel.y() / float(res.y()) * dy + window[1]
    );
}
