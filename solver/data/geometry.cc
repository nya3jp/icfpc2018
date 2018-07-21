#include "solver/data/geometry.h"

std::ostream& operator<<(std::ostream& os, Axis a) {
  return os << (a == Axis::X ? "X" : a == Axis::Y ? "Y" : "Z");
}

std::ostream& operator<<(std::ostream& os, const Delta& d) {
  return os << "Delta(" << d.dx << ", " << d.dy << ", " << d.dz << ")";
}

std::ostream& operator<<(std::ostream& os, const LinearDelta& d) {
  return os << "LinearDelta(" << d.axis << ", " << d.delta << ")";
}

std::ostream& operator<<(std::ostream& os, const Point& p) {
  return os << "Point(" << p.x << ", " << p.y << ", " << p.z << ")";
}

// static
Region Region::FromPointDelta(const Point& p, const Delta& d) {
  Region r;
  r.mini.x = std::min(p.x, p.x + d.dx);
  r.mini.y = std::min(p.y, p.y + d.dy);
  r.mini.z = std::min(p.z, p.z + d.dz);
  r.maxi.x = std::max(p.x, p.x + d.dx);
  r.maxi.y = std::max(p.y, p.y + d.dy);
  r.maxi.z = std::max(p.z, p.z + d.dz);
  r.Verify();
  return r;
}

void Region::Verify() const {
  CHECK(mini.x <= maxi.x && mini.y <= maxi.y && mini.z <= maxi.z) << *this;
}

std::ostream& operator<<(std::ostream& os, const Region& r) {
  return os << "Region(" << r.mini.x << ".." << r.maxi.x
            << ", " << r.mini.y << ".." << r.maxi.y
            << ", " << r.mini.z << ".." << r.maxi.z << ")";
}