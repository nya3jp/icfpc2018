#include "solver/impls/deleter3.h"

void DeleterStrategy3::Decide(TickExecutor::Commander *commander) {

  const Matrix &matrix = commander->field()->matrix();
  const std::map<int, BotState> &bots = commander->field()->bots();

  if (start_x_ < 0) {
    start_x_ = matrix.Resolution();
    end_x_ = 0;
    start_z_ = matrix.Resolution();
    end_z_ = 0;
    height_ = 0;
    for (int x = 0; x < matrix.Resolution(); ++x) {
      for (int y = 0; y < matrix.Resolution(); ++y) {
        for (int z = 0; z < matrix.Resolution(); ++z) {
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

  auto x_size = (end_x_ - start_x_ - 1) / 29 + 2;
  auto y_size = std::min(40 / x_size, (height_ - 1) / 29 + 2);



  if (state_ == State::FISSION) {
    // zをstart地点へ
    if (bots.size() == 1) {
      for (const auto &bot : bots) {
        if (bot.second.position().z < start_z_ - 1) {
          commander->Set(
              bot.second.id(),
              Command::SMove(LinearDelta(Axis::Z, std::min(15, start_z_ - bot.second.position().z - 1))));
          return;
        }
      }
    }
    // xをstart地点へ
    if (bots.size() == 1) {
      for (const auto &bot : bots) {
        if (bot.second.position().x < start_x_) {
          commander->Set(
              bot.second.id(),
              Command::SMove(LinearDelta(Axis::X, std::min(15, start_x_ - bot.second.position().x))));
          return;
        }
      }
    }

    // x方向に展開
    if (bots.size() < x_size) {
      for (const auto &bot : bots) {
        if (bot.second.position().x % 29 != start_x_ % 29) {
          commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::X, 14)));
          return;
        }
      }
      for (const auto &bot : bots) {
        auto seeds_size = __builtin_popcountl(bot.second.seeds());
        if (seeds_size > y_size) {
          commander->Set(bot.second.id(), Command::Fission(Delta(1, 0, 0), seeds_size - y_size));
          return;
        }
      }
    }
    // x方向、最後の一個
    for (const auto &bot : bots) {
      if (bot.second.position().x % 29 != start_x_ % 29 &&
          bot.second.position().x < end_x_) {
        commander->Set(bot.second.id(),
                       Command::SMove(LinearDelta(Axis::X, std::min(15, end_x_ - bot.second.position().x))));
        return;
      }
    }

    // y方向に展開
    if (bots.size() < x_size * y_size) {
      {
        bool any = false;
        for (const auto &bot : bots) {
          if (bot.second.position().y % 29 != 0) {
            bool set = commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::Y, 14)));
            any = true;
          }
        }
        if (any) { return; }
      }
      for (const auto &bot : bots) {
        auto seeds_size = __builtin_popcountl(bot.second.seeds());
        if (seeds_size >= 1) {
          commander->Set(bot.second.id(), Command::Fission(Delta(0, 1, 0), seeds_size - 1));
        }
      }
      return;
    }
    {
      // y方向、最後の一個
      bool any = false;
      for (const auto &bot : bots) {
        if (bot.second.position().y % 29 != 0 &&
            bot.second.position().y < height_) {
          commander->Set(bot.second.id(),
                         Command::SMove(LinearDelta(Axis::Y, std::min(14, height_ - bot.second.position().y))));
          any = true;
        }
      }

      if (any) { return; }
      //展開終了, Flipして次のturn
      state_ = State::DELETION;
      commander->Set(0, Command::Flip());
      dz_ = 1;
      return;
    }
  }
  if (state_ == State::DELETION) {

    auto current_pos = bots.find(0)->second.position();
    int cur_min_y = matrix.Resolution();
    int cur_max_y = 0;
    for (const auto &bot : bots) {
      cur_min_y = std::min(cur_min_y, bot.second.position().y);
      cur_max_y = std::max(cur_max_y, bot.second.position().y);
    }

    // 終了判定
    if (dz_ > 0 && current_pos.z == end_z_ + 1) {
      dz_ = -1;
      // 埋まっているならまずfissionして、その後(0,0,0)へ
      if (cur_max_y >= height_) {
        state_ = State::FUSION;
        commander->Set(0, Command::Flip());
        return;
      } else {
        state_ = State::ELEVATE;
        return Decide(commander);
      }
    } else if (dz_ < 0 && current_pos.z == start_z_ - 1) {
      dz_ = 1;
      // 埋まっているならまずfissionして、その後(0,0,0)へ
      if (cur_max_y >= height_) {
        state_ = State::FUSION;
        commander->Set(0, Command::Flip());
        return;
      } else {
        state_ = State::ELEVATE;
        return Decide(commander);
      }
    } else if (cur_max_y >= height_ && dz_ > 0 && current_pos.z == end_z_) {
      dz_ = -1;
      state_ = State::FUSION;
      commander->Set(0, Command::Flip());
      return;
    } else if (cur_max_y >= height_ && dz_ < 0 && current_pos.z == start_z_) {
      dz_ = 1;
      state_ = State::FUSION;
      commander->Set(0, Command::Flip());
      return;
    }

    // ここにいるということは消すturn
    // 領域を4つに分けて,順番に消して行く
    bool delete_odd_odd = false;
    bool delete_odd_even = false;
    bool delete_even_odd = false;
    bool delete_even_even = false;
    for (int x = start_x_; x <= end_x_; ++x) {
      for (int y = cur_min_y; y <= cur_max_y; ++y) {
        if (matrix.Get(x, y, current_pos.z + dz_)) {
          auto odd_x = ((x - start_x_) / 29) % 2 == 1;
          auto odd_y = ((y - cur_min_y) / 29) % 2 == 1;
          if (odd_x && odd_y) {
            delete_odd_odd = true;
          }
          if (odd_x && !odd_y) {
            delete_odd_even = true;
          }
          if (!odd_x && odd_y) {
            delete_even_odd = true;
          }
          if (!odd_x && !odd_y) {
            delete_even_even = true;
          }
        }
      }
    }
    // 一度に消す領域は一つまで
    if (delete_odd_odd) {
      delete_odd_even = delete_even_odd = delete_even_even = false;
    }
    if (delete_odd_even) {
      delete_even_odd = delete_even_even = false;
    }
    if (delete_even_odd) {
      delete_even_even = false;
    }
    // 消去パート
    if (delete_odd_odd || delete_odd_even || delete_even_odd || delete_even_even) {
      for (const auto &bot : bots) {
        auto x_pos = bot.second.position().x;
        auto y_pos = bot.second.position().y;
        auto x_p_s = x_pos - start_x_;
        auto y_p_s = y_pos - cur_min_y;
        auto x_odd = (x_p_s % 29 == 0 && (x_p_s / 29) % 2 == 1) || (x_p_s % 29 != 0 && (x_p_s / 29) % 2 == 0);
        auto y_odd = (y_p_s % 29 == 0 && (y_p_s / 29) % 2 == 1) || (y_p_s % 29 != 0 && (y_p_s / 29) % 2 == 0);

        int x_diff = 0;
        auto nd = Delta(0, 0, dz_);
        if ((x_odd && (delete_odd_odd || delete_odd_even)) || (!x_odd && (delete_even_odd || delete_even_even))) {
          // x は +
          x_diff = 29;
          if (x_pos == end_x_) {
            x_diff = 0;
          }
        } else {
          // x は -
          x_diff = -29;
          if (x_pos == start_x_) { continue; }
        }
        x_diff = std::min(x_diff, end_x_ - x_pos);
        x_diff = (x_p_s % 29 != 0 && x_diff < 0) ? -(x_p_s - (x_p_s / 29) * 29) : x_diff;

        int y_diff = 0;
        if ((y_odd && (delete_odd_odd || delete_even_odd)) || (!y_odd && (delete_odd_even || delete_even_even))) {
          // y は +
          y_diff = 29;
          if (y_pos == cur_max_y) {
            y_diff = 0;
          }
        } else {
          // y は -
          y_diff = -29;
          if (y_pos == cur_min_y) { continue; }
        }
        y_diff = std::min(y_diff, cur_max_y - bot.second.position().y);
        y_diff = (y_p_s % 29 != 0 && y_diff < 0) ? -(y_p_s - (y_p_s / 29) * 29) : y_diff;

        auto x_min = std::min(x_pos, x_pos + x_diff);
        auto x_max = std::max(x_pos, x_pos + x_diff);
        auto y_min = std::min(y_pos, y_pos + y_diff);
        auto y_max = std::max(y_pos, y_pos + y_diff);

        bool any_filled = false;
        for (int x = x_min; x <= x_max; ++x) {
          for (int y = y_min; y <= y_max; ++y) {
            if (matrix.Get(x, y, current_pos.z + dz_)) {
              any_filled = true;
            }
          }
        }
        if (!any_filled) { continue; }

        if (x_diff == 0 && y_diff == 0) {
          commander->Set(bot.second.id(), Command::Void(nd));
        } else {
          commander->Set(bot.second.id(), Command::GVoid(nd, Delta(x_diff, y_diff, 0)));
        }
      }
    } else {
      // dz_に進む
      for (const auto &bot : bots) {
        commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::Z, dz_)));
      }
    }
    return;
  }
  if (state_ == State::ELEVATE) {
    int cur_min_y = matrix.Resolution();
    int cur_max_y = 0;
    for (const auto &bot : bots) {
      cur_min_y = std::min(cur_min_y, bot.second.position().y);
      cur_max_y = std::max(cur_max_y, bot.second.position().y);
    }

    if (cur_max_y >= height_) {
      state_ = State::DELETION;
      return Decide(commander);
    }
    if (cur_max_y % (29 * (y_size - 1)) ||
        (dz_ < 0 && cur_min_y % (58 * (y_size - 1)) == 0) ||
        (dz_ > 0 && cur_max_y % (58 * (y_size - 1)) == 0)) {
      for (const auto &bot : bots) {
        if (bot.second.position().y % 29 == 15) {
          commander->Set(bot.second.id(),
                         Command::SMove(LinearDelta(Axis::Y, std::min(14, height_ - bot.second.position().y))));
        } else {
          commander->Set(bot.second.id(),
                         Command::SMove(LinearDelta(Axis::Y, std::min(15, height_ - bot.second.position().y))));
        }
      }
      return;
    } else {
      state_ = State::DELETION;
      return Decide(commander);
    }
    return;
  }
  if (state_ == State::FUSION) {

    { // y方向のfusion
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
      if (bots.size() > x_size) {
        bool any = false;
        for (const auto &bot : bots) {
          if (bot.second.position().y == cur_max_y) {
            if (bot.second.position().y != cur_next_y + 1) {
              // 移動
              auto target_y = cur_next_y + 1;
              commander->Set(bot.second.id(),
                             Command::SMove(LinearDelta(Axis::Y, std::max(-15, target_y - bot.second.position().y))));
              any = true;
            }
          }
        }
        if (any) { return; }
        for (const auto &bot : bots) {
          if (bot.second.position().y == cur_max_y) {
            commander->Set(bot.second.id(), Command::FusionS(Delta(0, -1, 0)));
          } else if (bot.second.position().y == cur_max_y - 1) {
            commander->Set(bot.second.id(), Command::FusionP(Delta(0, 1, 0)));
          }
        }
        return;
      }
    }
    { // x方向のfusion
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
      if (bots.size() > 1) {
        bool any = false;
        for (const auto &bot : bots) {
          if (bot.second.position().x == cur_max_x) {
            if (bot.second.position().x != cur_next_x + 1) {
              // 移動
              auto target_x = cur_next_x + 1;
              commander->Set(bot.second.id(),
                             Command::SMove(LinearDelta(Axis::X, std::max(-15, target_x - bot.second.position().x))));
              any = true;
            }
          }
        }
        if (any) { return; }
        for (const auto &bot : bots) {
          if (bot.second.position().x == cur_max_x) {
            commander->Set(bot.second.id(), Command::FusionS(Delta(-1, 0, 0)));
          } else if (bot.second.position().x == cur_max_x - 1) {
            commander->Set(bot.second.id(), Command::FusionP(Delta(1, 0, 0)));
          }
        }
        return;
      }
    }
    {
      // (0, 0, 0)へ
      for (const auto &bot : bots) {
        if (bot.second.position().x > 0) {
          commander->Set(bot.second.id(),
                         Command::SMove(LinearDelta(Axis::X, std::max(-15, -bot.second.position().x))));
          return;
        }
        if (bot.second.position().y > 0) {
          commander->Set(bot.second.id(),
                         Command::SMove(LinearDelta(Axis::Y, std::max(-15, -bot.second.position().y))));
          return;
        }
        if (bot.second.position().z > 0) {
          commander->Set(bot.second.id(),
                         Command::SMove(LinearDelta(Axis::Z, std::max(-15, -bot.second.position().z))));
          return;
        }
      }
    }
    // 終わり
    if (halt_) {
      for (const auto &bot : bots) {
        commander->Set(bot.second.id(), Command::Halt());
      }
    }
      // FIXME: とりあえず初期位置のbotのIDが0でないといけないassemblerがいるのでチェックする
    else {
      CHECK(bots.size() == 1 && bots.cbegin()->first == 0);
    }
    finished_ = true;
    return;
  }
}

DeleteStrategySolver3::DeleteStrategySolver3(
    const Matrix *source, const Matrix *target, TraceWriter *writer, bool halt)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())),
      writer_(writer),
      model_(source),
      halt_(halt) {
}

void DeleteStrategySolver3::Solve() {
  DeleterStrategy3 strategy(model_, halt_);
  TickExecutor executor(&strategy);
  while (!strategy.Finished()) {
    executor.Run(&field_, writer_);
  }
}
