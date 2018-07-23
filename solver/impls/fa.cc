#include "solver/impls/fa.h"

void FA::Decide(TickExecutor::Commander *commander) {

  const Matrix &matrix = commander->field()->matrix();
  const std::map<int, BotState> &bots = commander->field()->bots();

  if (start_x_ < 0) {
    start_x_ = model_->Resolution();
    end_x_ = 0;
    start_z_ = model_->Resolution();
    end_z_ = 0;
    height_ = 0;
    for (int x = 0; x < model_->Resolution(); ++x) {
      for (int y = 0; y < model_->Resolution(); ++y) {
        for (int z = 0; z < model_->Resolution(); ++z) {
          if (model_->Get(x, y, z)) {
            start_x_ = std::min(x, start_x_);
            end_x_ = std::max(x, end_x_);
            start_z_ = std::min(z, start_z_);
            end_z_ = std::max(z, end_z_);
            height_ = std::max(height_, y);
          }
        }
      }
    }
  }
  int cur_min_x = matrix.Resolution();
  int cur_max_x = 0;
  for (const auto &bot : bots) {
    cur_min_x = std::min(cur_min_x, bot.second.position().x);
    cur_max_x = std::max(cur_max_x, bot.second.position().x);
  }
  int cur_next_x = 0;
  for (const auto &bot : bots) {
    if (bot.second.position().x != cur_max_x) {
      cur_next_x = std::max(cur_next_x, bot.second.position().x);
    }
  }
  int cur_min_y = matrix.Resolution();
  int cur_max_y = 0;
  for (const auto &bot : bots) {
    cur_min_y = std::min(cur_min_y, bot.second.position().y);
    cur_max_y = std::max(cur_max_y, bot.second.position().y);
  }
  int cur_next_y = 0;
  for (const auto &bot : bots) {
    if (bot.second.position().y != cur_max_y) {
      cur_next_y = std::max(cur_next_y, bot.second.position().y);
    }
  }
  int cur_min_z = matrix.Resolution();
  int cur_max_z = 0;
  for (const auto &bot : bots) {
    cur_min_z = std::min(cur_min_z, bot.second.position().z);
    cur_max_z = std::max(cur_max_z, bot.second.position().z);
  }
  int cur_next_z = 0;
  for (const auto &bot : bots) {
    if (bot.second.position().z != cur_max_z) {
      cur_next_z = std::max(cur_next_z, bot.second.position().z);
    }
  }

  if (num_ == 15) {
    if (state_ == State::FISSION) {
      if (bots.size() == 0) { exit(1); }
      if (bots.size() == 1) {
        for (const auto &bot : bots) {
          if (bot.second.position().z < start_z_) {
            commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::Z, std::min(15, start_z_))));
            return;
          }
        }
        for (const auto &bot : bots) {
          if (bot.second.position().x < start_x_) {
            commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::X, std::min(15, start_x_))));
            return;
          }
        }
      }
      if (bots.size() <= (end_x_ - start_x_ + 1) / 2) {
        for (const auto &bot : bots) {
          if (bot.second.position().x == cur_max_x) {
            auto seed_size = __builtin_popcountl(bot.second.seeds());
            std::cout << "fission" << std::endl;
            commander->Set(bot.second.id(), Command::Fission(Delta(1, 0, 0), seed_size - (end_z_ - start_z_ + 1)));
          }
        }
        return;
      }
      if (bots.size() < (end_x_ - start_x_ + 1) / 2 * (end_z_ - start_z_ + 1)) {
        for (const auto &bot : bots) {
          if (bot.second.position().z == cur_max_z) {
            auto seed_size = __builtin_popcountl(bot.second.seeds());
            std::cout << "fission" << std::endl;
            commander->Set(bot.second.id(), Command::Fission(Delta(0, 0, 1), seed_size - 1));
          }
        }
        return;
      }
      if (start_x_ != cur_min_x) {
        std::cout << "gvgggggggggggg" << std::endl;
        exit(1);
      }
      if (start_z_ != cur_min_z) {
        std::cout << "gvgggzzzzzzzzz" << std::endl;
        exit(1);
      }
      if (end_z_ != cur_max_z) {
        std::cout << "gvgggzZZZZZZZz" << std::endl;
        exit(1);
      } else {
        std::cout << "ZZZZZZZZZZZZ" << cur_max_z << std::endl;
      }
      state_ = State::RETURN;
    }
    if (cur_min_x == start_x_) {
      std::cout << "______" << std::endl;
      if (cur_max_y <= height_ + 1) {
        std::cout << "debug0" << std::endl;
        bool any = false;
        for (const auto &bot : bots) {
          auto pos = bot.second.position();
          if (pos.y > 0 && !matrix.Get(pos.x, pos.y - 1, pos.z) && model_->Get(pos.x, pos.y - 1, pos.z)) {
            auto ret = commander->Set(bot.second.id(), Command::Fill(Delta(0, -1, 0)));
            if (pos.y - 1 == 18) {
              std::cout << "===================================================" << pos + Delta(0, -1, 0) << std::endl;
            }
            any = true;
          }
        }
        if (any) { return; }
        if (cur_max_y == height_ + 1) {
          int hoge = 0;
          for (const auto &bot : bots) {
            if (bot.second.position().x <= (end_x_ - start_x_ + 3) / 2 + start_x_) {
              hoge = std::max(hoge, bot.second.position().x);
            }
          }
          for (const auto &bot : bots) {
            if (bot.second.position().x == hoge) {
              commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::X, (end_x_ - start_x_ + 3) / 2)));
            }
          }
          return;
        } else {
          for (const auto &bot : bots) {
            commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::Y, 1)));
          }
          return;
        }
      } else {
        std::cout << "debug2" << std::endl;
      }
    } else {
      std::cout << "=====" << std::endl;
      if (cur_max_y > 0) {
        std::cout << "debug3" << std::endl;
        bool any = false;
        for (const auto &bot : bots) {
          auto pos = bot.second.position();
          if (pos.y + 1 < matrix.Resolution() && !matrix.Get(pos.x, pos.y + 1, pos.z) && model_->Get(pos.x, pos.y + 1, pos.z)) {
            commander->Set(bot.second.id(), Command::Fill(Delta(0, 1, 0)));
            any = true;
          }
        }
        if (any) { return; }
        std::cout << "debug4" << std::endl;
        for (const auto &bot : bots) {
          commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::Y, -1)));
        }
        return;
      } else {
        std::cout << "debug5" << std::endl;
        bool any = false;
        for (const auto &bot : bots) {
          auto pos = bot.second.position();
          if (!matrix.Get(pos.x, pos.y + 1, pos.z)&&model_->Get(pos.x, pos.y + 1, pos.z)) {
            commander->Set(bot.second.id(), Command::Fill(Delta(0, 1, 0)));
            any = true;
          }
        }
        if (any) { return; }
        std::cout << "debug6: " << bots.size() << std::endl;
        for (const auto &bot : bots) {
          std::cout << bot.second.id() << " " << bot.second.position() << std::endl;
        }
        std::cout << "debug6.5" << std::endl;
        if (cur_max_x > cur_min_x) {
          std::cout << "debug7" << std::endl;
          for (const auto &bot : bots) {
            if (bot.second.position().x == cur_max_x) {
//              std::cout << bot.second.id() << "FS " << bot.second.position() << std::endl;
              commander->Set(bot.second.id(), Command::FusionS(Delta(-1, 0, 0)));
            } else if (bot.second.position().x == cur_next_x) {
//              std::cout << bot.second.id() << "FP " << bot.second.position() << std::endl;
              commander->Set(bot.second.id(), Command::FusionP(Delta(1, 0, 0)));
            }
          }
          return;
        }
        if (cur_max_z > cur_min_z) {
          std::cout << "debug6 " << bots.size() << std::endl;
          for (const auto &bot : bots) {
            if (bot.second.position().z == cur_max_z) {
              std::cout << "FS " << bot.second.position() << std::endl;
              auto ret = commander->Set(bot.second.id(), Command::FusionS(Delta(0, 0, -1)));
              std::cout << ret << std::endl;
            } else if (bot.second.position().z == cur_next_z) {
              std::cout << "FP " << bot.second.position() << std::endl;
              auto ret = commander->Set(bot.second.id(), Command::FusionP(Delta(0, 0, 1)));
              std::cout << ret << std::endl;
            } else {

              std::cout << "HOGE " << bot.second.position() << std::endl;
            }
          }
          return;
        }
        {
          std::cout << "debug8" << std::endl;
          for (const auto &bot : bots) {
            if (bot.second.position().z > 0) {
              std::cout << "debug10" << std::endl;
              commander->Set(bot.second.id(),
                             Command::SMove(LinearDelta(Axis::Z, -std::min(15, bot.second.position().z))));
              return;
            }
            if (bot.second.position().x > 0) {
              std::cout << bots.size() << std::endl;
              std::cout << "debug9 " << bot.second.position().x << std::endl;
              commander->Set(bot.second.id(),
                             Command::SMove(LinearDelta(Axis::X, -std::min(15, bot.second.position().x))));
              return;
            }
            {
              std::cout << "debug11" << std::endl;
              commander->Set(bot.second.id(), Command::Halt());
              return;
            }
          }
        }
        return;
      }
    }
    std::cout << "debug-err" << std::endl;
    return;
  }

}

FASolver::FASolver(
    const Matrix *source, const Matrix *target, TraceWriter *writer, bool halt, int num)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())),
      writer_(writer),
      model_(target),
      halt_(halt),
      num_(num) {
}

void FASolver::Solve() {
  FA strategy(model_, halt_, num_);
  TickExecutor executor(&strategy);
  while (!field_.IsHalted()) {
    executor.Run(&field_, writer_);
  }
}

