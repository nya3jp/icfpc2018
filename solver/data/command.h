#ifndef SOLVER_DATA_COMMAND_H
#define SOLVER_DATA_COMMAND_H

#include "glog/logging.h"

#include "solver/data/geometry.h"

struct Command {
  enum Type {
    HALT,
    WAIT,
    FLIP,
    LMOVE,
    SMOVE,
    FILL,
    VOID,
    GFILL,
    GVOID,
    FISSION,
    FUSION_MASTER,
    FUSION_SLAVE,
  };

  Type type;
  LinearDelta ld1;
  LinearDelta ld2;
  Delta nd;
  Delta fd;
  int arg;

  Command() : type(WAIT), arg(0) {}
  explicit Command(Type type, LinearDelta ld1, LinearDelta ld2, Delta nd, Delta fd, int arg)
      : type(type), ld1(ld1), ld2(ld2), nd(nd), fd(fd), arg(arg) {}

  static Command Halt() {
    return Command(HALT, LinearDelta(), LinearDelta(), Delta(), Delta(), 0);
  }
  static Command Wait() {
    return Command(WAIT, LinearDelta(), LinearDelta(), Delta(), Delta(), 0);
  }
  static Command Flip() {
    return Command(FLIP, LinearDelta(), LinearDelta(), Delta(), Delta(), 0);
  }
  static Command LMove(const LinearDelta& ld1, const LinearDelta& ld2) {
    CHECK(ld1.IsShort());
    CHECK(ld2.IsShort());
    return Command(LMOVE, ld1, ld2, Delta(), Delta(), 0);
  }
  static Command SMove(const LinearDelta& ld) {
    CHECK(ld.IsLong());
    return Command(SMOVE, ld, LinearDelta(), Delta(), Delta(), 0);
  }
  static Command Fill(const Delta& nd) {
    CHECK(nd.IsNear());
    return Command(FILL, LinearDelta(), LinearDelta(), nd, Delta(), 0);
  }
  static Command Void(const Delta& nd) {
    CHECK(nd.IsNear());
    return Command(VOID, LinearDelta(), LinearDelta(), nd, Delta(), 0);
  }
  static Command GFill(const Delta& nd, const Delta& fd) {
    CHECK(nd.IsNear());
    CHECK(fd.IsFar());
    return Command(GFILL, LinearDelta(), LinearDelta(), nd, fd, 0);
  }
  static Command GVoid(const Delta& nd, const Delta& fd) {
    CHECK(nd.IsNear());
    CHECK(fd.IsFar());
    return Command(GVOID, LinearDelta(), LinearDelta(), nd, fd, 0);
  }
  static Command Fission(const Delta& nd, int arg) {
    CHECK(nd.IsNear());
    return Command(FISSION, LinearDelta(), LinearDelta(), nd, Delta(), arg);
  }
  static Command FusionP(const Delta& nd) {
    CHECK(nd.IsNear());
    return Command(FUSION_MASTER, LinearDelta(), LinearDelta(), nd, Delta(), 0);
  }
  static Command FusionS(const Delta& nd) {
    CHECK(nd.IsNear());
    return Command(FUSION_SLAVE, LinearDelta(), LinearDelta(), nd, Delta(), 0);
  }
};

#endif //SOLVER_DATA_COMMAND_H
