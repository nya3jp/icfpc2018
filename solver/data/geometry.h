#ifndef SOLVER_DATA_GEOMETRY_H
#define SOLVER_DATA_GEOMETRY_H

#include "glog/logging.h"

enum class Axis {
  X = 0,
  Y = 1,
  Z = 2,
};

struct Delta {
  int dx, dy, dz;

  Delta(int dx = 0, int dy = 0, int dz = 0) : dx(dx), dy(dy), dz(dz) {}
};

struct LinearDelta {
  Axis axis;
  int delta;

  operator Delta() const {
    switch (axis) {
      case Axis::X:
        return Delta{delta, 0, 0};
      case Axis::Y:
        return Delta{0, delta, 0};
      case Axis::Z:
        return Delta{0, 0, delta};
    }
  }
};

struct Point {
  int x, y, z;

  Point(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z) {}

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

#endif // SOLVER_DATA_GEOMETRY_H
