#ifndef SOLVER_DATA_ACTION_H
#define SOLVER_DATA_ACTION_H

#include "glog/logging.h"

#include "solver/data/geometry.h"

struct Action {
  enum Type {
    HALT,
    WAIT,
    FLIP,
    MOVE,
    FILL,
    VOID,
    FISSION,
    FUSION,
  };

  Type type;
  int bot_id;
  Point point;
  Region region;
  int arg;

  Action() : type(WAIT), arg(0) {}
  explicit Action(Type type, int bot_id, Point point, Region region, int arg)
      : type(type), bot_id(bot_id), point(point), region(region), arg(arg) {}

  static Action Halt(int bot_id) {
    return Action(HALT, bot_id, Point(), Region(), 0);
  }
  static Action Wait() {
    return Action(WAIT, 0, Point(), Region(), 0);
  }
  static Action Flip() {
    return Action(FLIP, 0, Point(), Region(), 0);
  }
  static Action Move(int bot_id, const Point& point, int energy) {
    return Action(MOVE, bot_id, point, Region(), energy);
  }
  static Action Fill(const Point& point) {
    return Action(FILL, 0, Point(), Region::FromPoint(point), 0);
  }
  static Action Fill(const Region& region) {
    return Action(FILL, 0, Point(), region, 0);
  }
  static Action Void(const Point& point) {
    return Action(VOID, 0, Point(), Region::FromPoint(point), 0);
  }
  static Action Void(const Region& region) {
    return Action(VOID, 0, Point(), region, 0);
  }
  static Action Fission(int bot_id, const Point& point, int nchildren) {
    return Action(FISSION, bot_id, point, Region(), nchildren);
  }
  static Action Fusion(int bot_id, int slave_id) {
    return Action(FUSION, bot_id, Point(), Region(), slave_id);
  }
};

#endif //SOLVER_DATA_ACTION_H
