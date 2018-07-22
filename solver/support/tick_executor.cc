#include "solver/support/tick_executor.h"

bool TickExecutor::Commander::Set(int bot_id, const Command& command) {
  const std::vector<Region> vcs = VolatileCoordinates(bot_id, command);
  if (command.type == Command::GFILL || command.type == Command::GVOID) {
    CHECK(vcs.size() == 3);
    const Region& current = vcs[0];
    const Region& nd = vcs[1];
    const Region& fd = vcs[2];
    if (Interfere(current)) {
      return false;
    }
    auto& map = command.type == Command::GFILL ? gfills_ : gvoids_;
    if (map.count(fd) == 0) {
      if (Interfere(fd)) {
        return false;
      }
      if (!field_->matrix().IsPlaceable(fd)) {
        return false;
      }
      map.insert(std::make_pair(fd, std::set<Region>{nd}));
    } else {
      map[fd].insert(nd);
    }
  } else if (command.type == Command::FUSION_MASTER || command.type == Command::FUSION_SLAVE) {
    CHECK(vcs.size() == 2);
    const Region& current = vcs[0];
    const Region& nd = vcs[1];
    if (Interfere(current)) {
      return false;
    }
    auto regions = command.type == Command::FUSION_MASTER ? std::make_pair(current, nd) : std::make_pair(nd, current);
    auto pair = std::make_pair(regions, bot_id);
    if (command.type == Command::FUSION_MASTER) {
      masters_.insert(pair);
    } else {
      slaves_.insert(pair);
    }
  } else {
    if (command.type == Command::LMOVE || command.type == Command::SMOVE) {
      for (const auto& vc : vcs) {
        if (!field_->matrix().IsMovable(vc)) {
          return false;
        }
      }
    } else if (command.type == Command::FILL || command.type == Command::VOID) {
      const Region& nd = vcs[1];
      if (!field_->matrix().IsPlaceable(nd)) {
        return false;
      }
    } else if (command.type == Command::FISSION) {
      const Region& nd = vcs[1];
      if (!field_->matrix().IsMovable(nd)) {
        return false;
      }
    }
    for (const auto& vc : vcs) {
      if (Interfere(vc)) {
        return false;
      }
    }
  }

  CHECK(commands_.count(bot_id) > 0);
  commands_[bot_id] = command;
  return true;
}

TickExecutor::Commander::Commander(const FieldState* field)
    : field_(field) {
  for (const auto& pair : field->bots()) {
    commands_[pair.first];  // Init with WAIT command.
  }
}

bool TickExecutor::Commander::Interfere(const Region& region){
  for (const auto& other : footprints_) {
    if (region.mini.x <= other.maxi.x && other.mini.x <= region.maxi.x &&
        region.mini.y <= other.maxi.y && other.mini.y <= region.maxi.y &&
        region.mini.z <= other.maxi.z && other.mini.z <= region.maxi.z) {
      return true;
    }
  }
  footprints_.push_back(region);
  return false;
}

const std::vector<Region> TickExecutor::Commander::VolatileCoordinates(int bot_id, const Command& command) {
  const BotState& bot = field_->bots().at(bot_id);
  const Point& current = bot.position();
  switch (command.type) {
    case Command::HALT:
    case Command::WAIT:
    case Command::FLIP:
      return {Region::FromPoint(current)};

    case Command::LMOVE:
      {
        const LinearDelta& sld1 = command.ld1;
        LinearDelta sld2_neighbor = command.ld2;
        LinearDelta sld2 = command.ld2;
        sld2_neighbor.delta /= std::abs(sld2_neighbor.delta);
        sld2.delta -= sld2_neighbor.delta;

        Point next = current + sld1.ToDelta() + sld2_neighbor.ToDelta();
        return {Region::FromPointDelta(current, sld1.ToDelta()),
                Region::FromPointDelta(next, sld2.ToDelta())};
      }

    case Command::SMOVE:
      return {Region::FromPointDelta(current, command.ld1.ToDelta())};

    case Command::FILL:
    case Command::VOID:
    case Command::FISSION:
    case Command::FUSION_MASTER:
    case Command::FUSION_SLAVE:
      return {Region::FromPoint(current), Region::FromPoint(current + command.nd)};

    case Command::GFILL:
    case Command::GVOID:
      {
        Point next = current + command.nd;
        return {Region::FromPoint(current), Region::FromPoint(next), Region::FromPointDelta(next, command.fd)};
      }
  }
}

const Point TickExecutor::Commander::Operand(int bot_id, const Command& command) {
  const BotState& bot = field_->bots().at(bot_id);
  const Point& current = bot.position();
  switch (command.type) {
    case Command::HALT:
    case Command::WAIT:
    case Command::FLIP:
    case Command::FUSION_MASTER:
    case Command::FUSION_SLAVE:
    case Command::GFILL:
    case Command::GVOID:
      return Point();

    case Command::LMOVE:
      return current + command.ld1.ToDelta() + command.ld2.ToDelta();

    case Command::SMOVE:
      return current + command.ld1.ToDelta();

    case Command::FILL:
    case Command::VOID:
    case Command::FISSION:
      return current + command.nd;
  }
}

const std::vector<Action> TickExecutor::Commander::GetAction() {
  std::vector<Action> actions;
  for (const auto& pair : commands_) {
    int bot_id = pair.first;
    const Command& command = pair.second;
    const Point target = Operand(bot_id, command);
    switch (command.type) {
      case Command::HALT:
        actions.push_back(Action::Halt(bot_id));
        break;

      case Command::WAIT:
        actions.push_back(Action::Wait());
        break;

      case Command::FLIP:
        actions.push_back(Action::Flip());
        break;

      case Command::LMOVE:
      case Command::SMOVE:
        actions.push_back(Action::Move(bot_id, target));
        break;

      case Command::FILL:
        actions.push_back(Action::Fill(target));
        break;

      case Command::VOID:
        actions.push_back(Action::Void(target));
        break;

      case Command::FISSION:
        actions.push_back(Action::Fission(bot_id, target, command.arg));
        break;

      case Command::FUSION_MASTER:
      case Command::FUSION_SLAVE:
      case Command::GFILL:
      case Command::GVOID:
        break;
    }
  }

  for (const auto& pair : gfills_) {
    const auto& region = pair.first;
    const auto& corners = pair.second;
    CHECK(corners.size() == (1 << region.Dimension()));
    actions.push_back(Action::Fill(region));
  }
  for (const auto& pair : gvoids_) {
    const auto& region = pair.first;
    const auto& corners = pair.second;
    CHECK(corners.size() == (1 << region.Dimension()));
    actions.push_back(Action::Void(region));
  }
  CHECK(masters_.size() == slaves_.size());
  for (const auto& pair : masters_) {
    const auto& regions = pair.first;
    const auto master_id = pair.second;
    CHECK(slaves_.count(regions) > 0);
    actions.push_back(Action::Fusion(master_id, slaves_[regions]));
  }
  return actions;
}

void TickExecutor::Run(FieldState* field, TraceWriter* writer) {
  Commander commander(field);
  strategy_->Decide(&commander);

  for (const auto& action : commander.GetAction()) {
    ApplyAction(field, action);
  }
  for (const auto& pair : commander.commands()) {
    writer->Command(pair.second);
  }
}

// static
void TickExecutor::ApplyAction(FieldState* field, const Action& action) {
  auto& bots = field->bots();
  switch (action.type) {
    case Action::HALT:
      CHECK(bots.size() == 1 && bots.at(action.bot_id).position().IsOrigin() && field->IsHarmonicsLow());
      bots.clear();
      break;

    case Action::WAIT:
      break;

    case Action::FLIP:
      field->FlipHarmonics();
      break;

    case Action::MOVE:
      // TODO: collision, hamidashi check
      bots.at(action.bot_id).set_position(action.point);
      break;

    case Action::FILL:
      field->matrix().Set(action.point.x, action.point.y, action.point.z, true);
      break;

    case Action::VOID:
      field->matrix().Set(action.point.x, action.point.y, action.point.z, false);
      break;

    case Action::FISSION:
      {
        int nchildren = action.arg;
        uint64_t seeds = bots.at(action.bot_id).seeds();
        CHECK(__builtin_popcountl(seeds) >= nchildren + 1);
        int new_bot_id = __builtin_ctzl(seeds);
        seeds ^= static_cast<uint64_t>(1) << new_bot_id;
        uint64_t new_seeds = 0;
        for (int i = 0; i < nchildren; ++i) {
          uint64_t bit = static_cast<uint64_t>(1) << __builtin_ctzl(seeds);
          new_seeds ^= bit;
          seeds ^= bit;
        }
        bots.at(action.bot_id).set_seeds(seeds);
        bots.insert(std::make_pair(new_bot_id, BotState(new_bot_id, new_seeds, action.point)));
        break;
      }

    case Action::FUSION:
      {
        int slave_id = action.arg;
        uint64_t seeds = bots.at(action.bot_id).seeds();
        seeds |= static_cast<uint64_t>(1) << slave_id;
        seeds |= bots.at(slave_id).seeds();
        bots.erase(slave_id);
        break;
      }
  }
}
