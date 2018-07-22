#ifndef SOLVER_DATA_COMMAND_H
#define SOLVER_DATA_COMMAND_H

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
};

#endif //SOLVER_DATA_COMMAND_H
