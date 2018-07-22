#include <fstream>
#include <iostream>
#include <queue>

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "solver/data/command.h"
#include "solver/data/geometry.h"
#include "solver/data/matrix.h"
#include "solver/io/model_reader.h"
#include "solver/io/trace_writer.h"
#include "solver/support/tick_executor.h"

class CommandIssuer : public TickExecutor::Strategy {
 public:
  CommandIssuer() = default;
  CommandIssuer(const CommandIssuer& other) = delete;

  void FromFile(const std::string& path);
  void Decide(TickExecutor::Commander* commander) override;
  bool IsCommandsEmpty() { return commands_.empty(); }
 private:
  std::queue<Command> commands_;
};

void CommandIssuer::Decide(TickExecutor::Commander* commander) {
  for (const auto& pair : commander->commands()) {
    int bot_id = pair.first;
    CHECK(commander->Set(bot_id, commands_.front()));
    commands_.pop();
  }
}

void CommandIssuer::FromFile(const std::string& path) {
  std::ifstream nbt(path);
  CHECK(nbt);

  while (true) {
    uint8_t code0;
    uint8_t code1;
    nbt.read(reinterpret_cast<char*>(&code0), 1);
    if (nbt.eof()) {
      break;
    }
    if (code0 == 0xff) {
      commands_.push(Command::Halt());
    } else if (code0 == 0xfe) {
      commands_.push(Command::Wait());
    } else if (code0 == 0xfd) {
      commands_.push(Command::Flip());
    } else {
      uint8_t subcode = code0 & 7;
      if (subcode == 4) {
        nbt.read(reinterpret_cast<char*>(&code1), 1);
        int axis1 = (int)((code0 >> 4) & 3) - 1;
        int axis2 = (int)((code0 >> 6) & 3) - 1;
        if (code0 & 8) {
          int delta1 = (int)(code1 & 0xf) - SHORT_LEN;
          int delta2 = (int)((code1 >> 4) & 0xf) - SHORT_LEN;
          auto ld1 = LinearDelta(static_cast<Axis>(axis1), delta1);
          auto ld2 = LinearDelta(static_cast<Axis>(axis2), delta2);
          commands_.push(Command::LMove(ld1, ld2));
        } else {
          int delta1 = (int)code1 - LONG_LEN;
          auto ld1 = LinearDelta(static_cast<Axis>(axis1), delta1);
          commands_.push(Command::SMove(ld1));
        }
      } else {
        int ndcode = code0 >> 3;
        int dx = ndcode / 9 - 1;
        int dy = ndcode % 9 / 3 - 1;
        int dz = ndcode % 3 - 1;
        auto nd = Delta(dx, dy, dz);
        if (subcode == 7) {
          commands_.push(Command::FusionP(nd));
        } else if (subcode == 6) {
          commands_.push(Command::FusionS(nd));
        } else if (subcode == 5) {
          nbt.read((char *)&code1, 1);
          commands_.push(Command::Fission(nd, code1));
        } else if (subcode == 3) {
          commands_.push(Command::Fill(nd));
        } else if (subcode == 2) {
          commands_.push(Command::Void(nd));
        } else {
          nbt.read((char *)&code1, 1);
          int dx = (int)code1 - 30;
          nbt.read((char *)&code1, 1);
          int dy = (int)code1 - 30;
          nbt.read((char *)&code1, 1);
          int dz = (int)code1 - 30;
          auto fd = Delta(dx, dy, dz);
          if (subcode == 1) {
            commands_.push(Command::GFill(nd, fd));
          } else {
            commands_.push(Command::GVoid(nd, fd));
          }
        }
      }
    }
  }
}

class Simulator {
 public:
  Simulator(const Matrix* source, const Matrix* target, CommandIssuer* trace)
    : field_(FieldState::FromModels(source->Copy(), target->Copy())), trace_(trace) {}
  void ReadCommands(std::ifstream ifs);
  void Exec();

  class DummyWriter : public TraceWriter {
   public:
    void Command(const struct Command& command) {}
  };

 private:
  FieldState field_;
  DummyWriter writer_;
  CommandIssuer* trace_;
};

void Simulator::Exec() {
  TickExecutor executor(trace_);
  int tick = 0;
  while (!field_.IsHalted()) {
    executor.Run(&field_, &writer_);
    ++tick;
  }
  CHECK(trace_->IsCommandsEmpty());
  CHECK(field_.matrix() == field_.target());
  std::cout << tick << " " << field_.energy() << std::endl;
}

DEFINE_string(source, "", "Path to source model file");
DEFINE_string(target, "", "Path to target model file");
DEFINE_string(trace, "", "Path to input trace file");

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::LogToStderr();
  google::InstallFailureSignalHandler();

  if ((FLAGS_source.empty() && FLAGS_target.empty()) || FLAGS_trace.empty()) {
    LOG(ERROR) << "Usage: " << argv[0] << " [--source=FR000_src.mdl] [--target=FR000_tgt.mdl] --trace=FR000.nbt";
    return 1;
  }

  Matrix source, target;
  if (!FLAGS_source.empty()) {
    source = ReadModel(FLAGS_source);
  }
  if (!FLAGS_target.empty()) {
    target = ReadModel(FLAGS_target);
  }

  if (source.IsZeroSized()) {
    source = Matrix::FromResolution(target.Resolution());
  }
  if (target.IsZeroSized()) {
    target = Matrix::FromResolution(source.Resolution());
  }

  CommandIssuer trace;
  trace.FromFile(FLAGS_trace);

  Simulator simulator(&source, &target, &trace);
  simulator.Exec();

  return 0;
}
