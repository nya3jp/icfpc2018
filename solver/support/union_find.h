#ifndef SOLVER_SUPPORT_UNION_FIND_H
#define SOLVER_SUPPORT_UNION_FIND_H

#include <map>

#include "solver/data/geometry.h"

class UnionFind {
 public:
  bool Add(Point p);
  bool Has(Point p);
  bool Connect(Point p1, Point p2);

  int CountRoots() const { return num_roots_; }

 private:
  struct Node {
    enum Type {
      ROOT,
      CHILD,
    };
    Type type;
    union {
      Point parent;
      int size;
    };

    Node() : type(ROOT), size(-1) {}
  };
  Point FindRoot(const Point& p);

  std::map<Point, Node> trees_;
  int num_roots_ = 0;
};

#endif //SOLVER_SUPPORT_UNION_FIND_H
