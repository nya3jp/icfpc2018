#include "solver/data/state.h"

#include <utility>

#include "glog/logging.h"

constexpr int NUM_BOTS = 40;

BotState::BotState(int id, uint64_t seeds, Point position)
    : id_(id), seeds_(seeds), position_(position) {
  CHECK((seeds & (static_cast<uint64_t>(1) << id)) == 0);
}

std::string BotState::ToJSON() const {
  std::string s = "{";
  s += "\"id\":" + std::to_string(id_) + ",";
  s += "\"seeds\":" + std::to_string(seeds_) + ",";
  s += "\"position\":[" + std::to_string(position_.x) + "," +
    std::to_string(position_.y) + "," +
    std::to_string(position_.z) + "]";
  s += "}";
  return s;
}

// static
FieldState FieldState::FromModels(Matrix source, Matrix target) {
  std::map<int, BotState> bots;
  BotState bot(0, (static_cast<uint64_t>(1) << NUM_BOTS) - 2, Point(0, 0, 0));
  bots.emplace(0, std::move(bot));
  return FieldState(std::move(target), std::move(source), std::move(bots));
}

std::string FieldState::ToJSON() const {
  std::string s = "{";
  s += "\"energy\":";
  s += std::to_string(energy_) + ",";
  s += "\"is_harmonics_low\":";
  s += IsHarmonicsLow() ? "true," : "false,";
  s += "\"bots\":[";
  for (const auto& pair : bots_) {
    std::string s_bot = "{";
    s_bot += "\"bot_id\":" + std::to_string(pair.first) + ",";
    s_bot += "\"bot_state\":" + pair.second.ToJSON();
    s_bot += "},";
    s += s_bot;
  }
  if (bots_.size() > 0) {
    s.pop_back();
  }
  s += "],";
  s += "\"matrix\":";
  s += matrix_.ToJSON();
  s += "}";
  return s;
}
