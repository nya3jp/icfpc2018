#include "solver/data/state.h"

#include <utility>

#include "glog/logging.h"

constexpr int NUM_BOTS = 40;

BotState::BotState(int id, uint64_t seeds, Point position)
    : id_(id), seeds_(seeds), position_(position) {
  CHECK((seeds & (static_cast<uint64_t>(1) << id)) != 0);
}

// static
FieldState FieldState::FromModels(Matrix source, Matrix target) {
  std::map<int, BotState> bots;
  BotState bot(1, (static_cast<uint64_t>(1) << NUM_BOTS) - 2, Point(0, 0, 0));
  bots.emplace(1, std::move(bot));
  return FieldState(std::move(target), std::move(source), std::move(bots));
}
