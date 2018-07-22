#ifndef SOLVER_DATA_GEOMETRY_H
#define SOLVER_DATA_GEOMETRY_H

#include <algorithm>
#include <iostream>

#include "glog/logging.h"

constexpr int SHORT_LEN = 5;
constexpr int LONG_LEN = 15;
constexpr int FAR_LEN = 30;

// Represents an axis.
enum class Axis {
  X = 0,
  Y = 1,
  Z = 2,
};

std::ostream& operator<<(std::ostream& os, Axis a);

// Represents a diff of two points.
struct Delta {
  int dx, dy, dz;

  explicit Delta(int dx = 0, int dy = 0, int dz = 0) : dx(dx), dy(dy), dz(dz) {}

  inline bool IsZero() const {
    return dx == 0 && dy == 0 && dz == 0;
  }
  inline int Manhattan() const {
    return std::abs(dx) + std::abs(dy) + std::abs(dz);
  }
  inline int Chessboard() const {
    return std::max(std::abs(dx), std::max(std::abs(dy), std::abs(dz)));
  }
  inline bool IsNear() const {
    return !IsZero() && Manhattan() <= 2 && Chessboard() == 1;
  }
  inline bool IsFar() const {
    return !IsZero() && Chessboard() <= FAR_LEN;
  }
};

std::ostream& operator<<(std::ostream& os, const Delta& d);

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
    return delta != 0 && std::abs(delta) <= SHORT_LEN;
  }
  inline bool IsLong() const {
    return delta != 0 && std::abs(delta) <= LONG_LEN;
  }
};

std::ostream& operator<<(std::ostream& os, const LinearDelta& d);

// Represents a point in 3D space.
struct Point {
  int x, y, z;

  explicit Point(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z) {}

  inline bool IsOrigin() const {
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

std::ostream& operator<<(std::ostream& os, const Point& p);

// Represents a region in 3D space.
struct Region {
  Point mini, maxi;

  Region() = default;
  static Region FromPoint(const Point& p);
  static Region FromPointDelta(const Point& p, const Delta& d);

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

  void Verify() const;

 private:
  Region(const Point& mini, const Point& maxi) : mini(mini), maxi(maxi) {
    Verify();
  }
};

std::ostream& operator<<(std::ostream& os, const Region& r);

#endif // SOLVER_DATA_GEOMETRY_H
