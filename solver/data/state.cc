#include "solver/data/state.h"

#include "glog/logging.h"

BotState::BotState(int id, uint64_t seeds, Point position)
    : id_(id), seeds_(seeds), position_(position) {
  CHECK((seeds & (static_cast<uint64_t>(1) << id)) != 0);
}
