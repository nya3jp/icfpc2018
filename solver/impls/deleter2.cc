#include "solver/impls/deleter.h"

#include "glog/logging.h"

void DeleterStrategy::Decide(TickExecutor::Commander *commander) {

  const Matrix &matrix = commander->field()->matrix();
  const std::map<int, BotState> &bots = commander->field()->bots();

  if (matrix.Resolution() > 29 * 6) {
    commander->Set(0, Command::Halt());
    return;
  }

  auto size = (matrix.Resolution() + 27) / 29 + 1;
  if (state_ == State::FISSION) {
    //まず必要な高さへ行く
    if (height_ < 0) {
      height_ = 0;
      for (int y = 0; y < model_->Resolution(); ++y) {
        for (int x = 0; x < model_->Resolution(); ++x) {
          for (int z = 0; z < model_->Resolution(); ++z) {
            if (model_->Get(x, y, z)) {
              height_ = y + 1;
              continue;
            }
          }
          if (height_ == y + 1) { continue; }
        }
      }
    }
    //x方向へ展開
    if (bots.size() == 1) {
      auto pos_y = bots.find(0)->second.position().y;
      if (bots.find(0)->second.position().y < height_) {
        //height_ に移動
        commander->Set(0, Command::SMove(LinearDelta(Axis::Y, std::min(15, height_ - pos_y))));
        return;
      }
      commander->Set(0, Command::Fission(Delta(1, 0, 0), 40 - 7));
      return;
    }
    if (bots.size() < size) {
      for (const auto &bot : bots) {
        if (bot.second.position().x % 29 != 0) {
          commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::X, 14)));
          return;
        }
      }
      for (const auto &bot : bots) {
        if (bot.second.position().x == (bots.size() - 1) * 29) {
          commander->Set(bot.second.id(), Command::Fission(Delta(1, 0, 0), 40 - 7 * bots.size()));
          return;
        }
      }
    }
    //x方向へ展開(最後, R-1まで行く)
    if (bots.size() == size) {
      for (const auto &bot : bots) {
        if (bot.second.position().x % 29 != 0) {
          if (bot.second.position().x != matrix.Resolution() - 1) {
            commander->Set(
                bot.second.id(),
                Command::SMove(
                    LinearDelta(Axis::X, std::min(15, matrix.Resolution() - 1 - bot.second.position().x))));
            return;
          }
        }
      }
    }
    //z方向へ展開
    if (bots.size() < size * size) {
      auto z_size = bots.size() / size;
      bool any = false;
      for (const auto &bot : bots) {
        if (bot.second.position().z % 29 != 0) {
          any = true;
          commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::Z, 14)));
        }
      }
      if (any) { return; }
      for (const auto &bot : bots) {
        if (bot.second.position().z == 29 * (z_size - 1)) {
          commander->Set(bot.second.id(), Command::Fission(Delta(0, 0, 1), 5 - z_size));
        }
      }
      return;
    }
    {
      //z方向へ展開(最後, R-1まで行く)
      bool any = false;
      for (const auto &bot : bots) {
        if (bot.second.position().z % 29 != 0 && bot.second.position().z != matrix.Resolution() - 1) {
          any = true;
          commander->Set(bot.second.id(),
                         Command::SMove(LinearDelta(Axis::Z,
                                                    std::min(15, matrix.Resolution() - 1 - bot.second.position().z))));
        }
      }
      if (any) { return; }
      //展開終了, Flipして次のturn
      state_ = State::DELETION;
      commander->Set(0, Command::Flip());
      return;
    }
  }
  if (state_ == State::DELETION) {
    auto current_pos = bots.find(0)->second.position();
    if (current_pos.y == 0) {  // もう消せない
      commander->Set(0, Command::Flip());
      state_ = State::FUSION;
      return;
    }
    // 領域を4つに分けて,順番に消して行く
    bool delete_odd_odd = false;
    bool delete_odd_even = false;
    bool delete_even_odd = false;
    bool delete_even_even = false;
    for (int x = current_pos.x; x < matrix.Resolution(); ++x) {
      for (int z = 0; z < matrix.Resolution(); ++z) {
        if (matrix.Get(x, current_pos.y - 1, z)) {
          if ((z / 29) % 2 && (x / 29) % 2) {
            delete_odd_odd = true;
          } else if ((z / 29) % 2 && (x / 29) % 2 == 0) {
            delete_even_odd = true;
          } else if ((x / 29) % 2) {
            delete_odd_even = true;
          } else {
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
        auto z_pos = bot.second.position().z;
        bool z_odd = (z_pos % 29 == 0 && (z_pos / 29) % 2 == 1) || (z_pos % 29 != 0 && (z_pos / 29) % 2 == 0);
        bool x_odd = (x_pos % 29 == 0 && (x_pos / 29) % 2 == 1) || (x_pos % 29 != 0 && (x_pos / 29) % 2 == 0);

        int x_diff = 0;
        auto nd = Delta(0, -1, 0);
        if ((x_odd && (delete_odd_odd || delete_odd_even)) || (!x_odd && (delete_even_odd || delete_even_even))) {
          // x は +
          x_diff = 29;
          if (x_pos == matrix.Resolution() - 1) { continue; }
        } else {
          // x は -
          x_diff = -29;
          if (x_pos == 0) { continue; }
        }
        x_diff = std::min(x_diff, matrix.Resolution() - bot.second.position().x - 1);
        x_diff = bot.second.position().x % 29 != 0 ? -(bot.second.position().x
            - (bot.second.position().x / 29) * 29) : x_diff;

        int z_diff = 0;
        if ((z_odd && (delete_odd_odd || delete_even_odd)) || (!z_odd && (delete_odd_even || delete_even_even))) {
          // z は +
          z_diff = 29;
          if (z_pos == matrix.Resolution() - 1) { continue; }
        } else {
          // z は -
          z_diff = -29;
          if (z_pos == 0) { continue; }
        }
        z_diff = std::min(z_diff, matrix.Resolution() - bot.second.position().z - 1);
        z_diff = bot.second.position().z % 29 != 0 ? -(bot.second.position().z
            - (bot.second.position().z / 29) * 29) : z_diff;

        commander->Set(bot.second.id(), Command::GVoid(nd, Delta(x_diff, 0, z_diff)));
      }
    } else {
      //下に下がる
      for (const auto &bot : bots) {
        commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::Y, -1)));
      }
    }
    return;
  }
  if (state_ == State::FUSION) {
    { // z方向のfusion
      if (bots.size() > size) {
        bool any = false;
        int z_size = bots.size() / size - 2;
        for (const auto &bot : bots) {
          if (bot.second.position().z > z_size * 29 + 1) {
            auto len = std::min(bot.second.position().z - z_size * 29 - 1, 15);
            commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::Z, -len)));
            any = true;
          }
        }
        if (any) { return; }
        for (const auto &bot : bots) {
          if (bot.second.position().z ==  z_size * 29 + 1) {
            commander->Set(bot.second.id(), Command::FusionS(Delta(0, 0, -1)));
          } else if (bot.second.position().z ==  z_size * 29) {
            commander->Set(bot.second.id(), Command::FusionP(Delta(0, 0, 1)));
          }
        }
        return;
      }
    }
    { // x方向のfusion
      if (bots.size() > 1) {
        int x_size = bots.size() - 2;
        for (const auto &bot : bots) {
          if (bot.second.position().x > x_size * 29 + 1) {
            auto len = std::min(bot.second.position().x - x_size * 29 - 1, 15);
            commander->Set(bot.second.id(), Command::SMove(LinearDelta(Axis::X, -len)));
            return;
          }
        }
        for (const auto &bot : bots) {
          if (bot.second.position().x ==  x_size * 29 + 1) {
            commander->Set(bot.second.id(), Command::FusionS(Delta(-1, 0, 0)));
          } else if (bot.second.position().x ==  x_size * 29) {
            commander->Set(bot.second.id(), Command::FusionP(Delta(1, 0, 0)));
          }
        }
        return;
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

DeleteStrategySolver::DeleteStrategySolver(
    const Matrix *source, const Matrix *target, TraceWriter *writer, bool halt)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())),
      model_(source),
      writer_(writer),
      halt_(halt) {
}

void DeleteStrategySolver::Solve() {
  DeleterStrategy strategy(model_, halt_);
  TickExecutor executor(&strategy);
  while (!strategy.Finished()) {
    executor.Run(&field_, writer_);
  }
}

