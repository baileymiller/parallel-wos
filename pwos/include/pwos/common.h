#pragma once

#include <pwos/fwd.h>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <map>
#include <math.h>
#include <memory>
#include <mutex>
#include <omp.h>
#include <pcg32.h>
#include <random>
#include <string>
#include <deque>
#include <vector>
#include <mutex>
#include <numeric>
#include <algorithm>

using std::mutex;
using std::string;
using std::unique_ptr;
using std::make_unique;
using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::deque;
using std::cout;
using std::endl;
using std::map;
using std::ifstream;
using std::stringstream;
using std::getline;
using std::stof;
using std::to_string;

// Macros / Constants
#define _USE_MATH_DEFINES

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;

typedef Eigen::Vector<float, Eigen::Dynamic> Vectorf;
typedef Eigen::Vector<int, 2> Vec2i;
typedef Eigen::Vector<float, 2> Vec2f;
typedef Eigen::Vector<float, 3> Vec3f;
typedef Eigen::Vector<float, 4> Vec4f;

typedef std::function<Vec3f(Vec2f, pcg32&)> Rand2DFunction;
typedef std::function<void()> FunctionBlock;

inline void THROW(string message)
{
    throw std::runtime_error(message);
}

inline void THROW_IF(bool cond, string message)
{
    if (cond) throw std::runtime_error(message);
}

inline void WARN_IF(bool cond, string message)
{
    if (cond) std::cerr << message << std::endl;
}

// epsilon used for general purpose calculations
#define EPSILON 1e-6

// the epsilon shell around boundary used to terminate random walks
#define BOUNDARY_EPSILON 1e-2

//========================//
// Types                  //
//========================//
enum ArgType
{
    INT,
    STR,
    VEC4f,
    VEC2i,
    FLOAT
};

enum RelativePositionType
{
    LEFT,
    RIGHT,
    INSIDE,
    ABOVE,
    BELOW
};

enum IntegratorType
{
    MCWOG,
    MCWOG_VISUAL,
    WOG,
    WOG_VISUAL,
    GRID_VISUAL,
    DISTANCE,
    WOS
};

const map<string, IntegratorType> StrToIntegratorType({
    { "mcwog", IntegratorType::MCWOG },
    { "mcwogviz", IntegratorType::MCWOG_VISUAL },
    { "wog", IntegratorType::WOG },
    { "wogviz", IntegratorType::WOG_VISUAL },
    { "gridviz", IntegratorType::GRID_VISUAL },
    { "dist", IntegratorType::DISTANCE },
    { "wos", IntegratorType::WOS }
});

enum class StatTimerType
{
    TOTAL,
    GRID_CREATION,
    SEND_WALKS,
    RECV_WALKS,
    CLOSEST_POINT_GRID,
    CLOSEST_POINT_QUERY,
    SETUP_CLOSEST_POINT_QUERY,
    SETUP
};

enum class StatType
{
    GRID_POINTS,
    CLOSEST_POINT_QUERY,
    SETUP_CLOSEST_POINT_QUERY,
    GRID_QUERY
};

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

inline Vec2f getXYCoords(Vec2i pixel, Vec4f window, Vec2i res)
{
    float dx = window[2] - window[0];
    float dy = window[3] - window[1];
    return Vec2f(
        pixel.x() / float(res.x()) * dx + window[0],
        (res.y() - pixel.y() - 1) / float(res.y()) * dy + window[1]
    );
}

inline Vec2i getPixelCoords(Vec2f p, Vec4f window, Vec2i res)
{
    float dx = window[2] - window[0];
    float dy = window[3] - window[1];
    float x = p.x() - window[0];
    float y = p.y() - window[1];
    int ix = (x / dx) * res.x();
    int iy = res.y() - (y / dy) * res.y() - 1;
    return Vec2i(ix, iy);
}

#define CLOSEST_POINT_QUERIES "closest_point_queries"
#define CLOSEST_POINT_GRID_QUERIES "closest_point_grid_queries"
