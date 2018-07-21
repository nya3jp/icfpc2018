#ifndef SOLVER_DATA_GEOMETRY_H
#define SOLVER_DATA_GEOMETRY_H

#include <algorithm>

#include "glog/logging.h"

// Represents an axis.
enum class Axis {
  X = 0,
  Y = 1,
  Z = 2,
};

// Represents a diff of two points.
struct Delta {
  int dx, dy, dz;

  explicit Delta(int dx = 0, int dy = 0, int dz = 0) : dx(dx), dy(dy), dz(dz) {}
};

constexpr int SHORT_LEN = 5;
constexpr int LONG_LEN = 15;
constexpr int FAR_LEN = 30;

// Represents something like Delta but parallel to an axis.
struct LinearDelta {
  Axis axis;
  int delta;

  explicit LinearDelta(Axis axis = Axis::X, int delta = 0)
      : axis(axis), delta(delta) {}

  Delta ToDelta() const {
    switch (axis) {
      case Axis::X:
        return Delta{delta, 0, 0};
      case Axis::Y:
        return Delta{0, delta, 0};
      case Axis::Z:
        return Delta{0, 0, delta};
    }
  }

  inline bool IsShort() const {
    return std::abs(delta) <= SHORT_LEN;
  }
  inline bool IsLong() const {
    return std::abs(delta) <= LONG_LEN;
  }
};

// Represents a point in 3D space.
struct Point {
  int x, y, z;

  explicit Point(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z) {}

  const inline bool IsOrigin() {
    return x == 0 && y == 0 && z == 0;
  }

  inline Point& operator+=(const Delta& o) {
    x += o.dx;
    y += o.dy;
    z += o.dz;
    return *this;
  }

  inline Point& operator-=(const Delta& o) {
    x -= o.dx;
    y -= o.dy;
    z -= o.dz;
    return *this;
  }

  inline Point operator+(const Delta& o) const {
    return Point(*this) += o;
  }

  inline Point operator-(const Delta& o) const {
    return Point(*this) -= o;
  }

  inline Delta operator-(const Point& o) const {
    return Delta{x - o.x, y - o.y, z - o.z};
  }

  inline bool operator==(const Point& o) const {
    return x == o.x && y == o.y && z == o.z;
  }

  inline bool operator!=(const Point& o) const {
    return !(*this == o);
  }

  inline bool operator<(const Point& o) const {
    return x < o.x || (x == o.x && (y < o.y || (y == o.y && z < o.z)));
  }
};

// Represents a region in 3D space.
struct Region {
  Point mini, maxi;

  explicit Region(Point& p, Delta& d) {
    mini.x = std::min(p.x, p.x + d.dx);
    mini.y = std::min(p.y, p.y + d.dy);
    mini.z = std::min(p.z, p.z + d.dz);
    maxi.x = std::max(p.x, p.x + d.dx);
    maxi.y = std::max(p.y, p.y + d.dy);
    maxi.z = std::max(p.z, p.z + d.dz);
  }

  inline int Dimension() const {
    return (mini.x < maxi.x ? 1 : 0) + (mini.y < maxi.y ? 1 : 0) + (mini.z < maxi.z ? 1 : 0);
  }

  inline bool operator==(const Region& o) const {
    return mini == o.mini && maxi == o.maxi;
  }

  inline bool operator!=(const Region& o) const {
    return !(*this == o);
  }

  inline bool operator<(const Region& o) const {
    return mini < o.mini || (mini == o.mini && maxi < o.maxi);
  }
};

#endif // SOLVER_DATA_GEOMETRY_H
