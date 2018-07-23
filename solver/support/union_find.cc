#include "solver/support/union_find.h"

#include <utility>

#include "glog/logging.h"

bool UnionFind::Add(Point p) {
  bool added = trees_.insert(std::make_pair(p, Node())).second;
  if (added) {
    ++num_roots_;
  }
  return added;
}

bool UnionFind::Has(Point p) {
  return trees_.count(p) > 0;
}

bool UnionFind::Connect(Point p1, Point p2) {
  if (!Has(p1) || !Has(p2)) {
    return false;
  }

  p1 = FindRoot(p1);
  p2 = FindRoot(p2);
  if (p1 == p2) {
    return false;
  }

  Node& node1 = trees_[p1];
  Node& node2 = trees_[p2];
  if (node1.size < node2.size) {
    node2.size += node1.size;
    node1.type = Node::CHILD;
    node1.parent = p2;
  } else {
    node1.size += node2.size;
    node2.type = Node::CHILD;
    node2.parent = p1;
  }
  --num_roots_;
  return true;
}

Point UnionFind::FindRoot(const Point& p) {
  auto iter = trees_.find(p);
  CHECK(iter != trees_.end());
  Node& node = iter->second;
  if (node.type == Node::ROOT) {
    return p;
  }
  Point root = FindRoot(node.parent);
  node.parent = root;
  return root;
}
