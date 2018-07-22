#ifndef SOLVER_SUPPORT_TICK_EXECUTOR_H
#define SOLVER_SUPPORT_TICK_EXECUTOR_H

#include <vector>

#include "solver/data/command.h"
#include "solver/data/state.h"
#include "solver/io/trace_writer.h"

class TickExecutor {
 public:
  class Commander {
   public:
    bool Set(int bot_id, const Command& command);

    const FieldState* field() const { return field_; }
    const std::map<int, Command>& commands() const { return commands_; }

   protected:
    explicit Commander(const FieldState* field);

   private:
    const FieldState* const field_;
    std::map<int, Command> commands_;

    friend class TickExecutor;
  };

  class Strategy {
   public:
    virtual void Decide(Commander* commander) = 0;
    
   protected:
    virtual ~Strategy() = default;
  };

  explicit TickExecutor(Strategy* strategy) : strategy_(strategy) {}
  TickExecutor(const TickExecutor& other) = delete;

  void Run(FieldState* field, TraceWriter* writer);

 private:
  static void ApplyCommand(int bot_id, const Command& command, FieldState* field);

  Strategy* const strategy_;
};

#endif //SOLVER_SUPPORT_TICK_EXECUTOR_H
