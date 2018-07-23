#ifndef SOLVER_SUPPORT_TICK_EXECUTOR_H
#define SOLVER_SUPPORT_TICK_EXECUTOR_H

#include <set>
#include <utility>
#include <vector>

#include "solver/data/action.h"
#include "solver/data/command.h"
#include "solver/data/state.h"
#include "solver/io/trace_writer.h"

class TickExecutor {
 public:
  class Commander {
   public:
    Commander(const Commander& other) = delete;
    Commander(Commander&& other);
    Commander& operator=(Commander&& other);

    bool Set(int bot_id, const Command& command);

    const FieldState* field() const { return field_; }
    const std::map<int, Command>& commands() const { return commands_; }
    const std::vector<Action> GetAction();
    std::vector<Region> GetFills() const;

    Commander Copy() const;

   protected:
    explicit Commander(const FieldState* field);

   private:
    bool Interfere(const Region& region);
    const std::vector<Region> VolatileCoordinates(int bot_id, const Command& command);
    const Point Operand(int bot_id, const Command& command);

    const FieldState* field_;
    std::map<int, Command> commands_;
    std::vector<Region> footprints_;
    std::vector<Region> fills_;
    std::map<Region, std::set<Region> > gfills_;
    std::map<Region, std::set<Region> > gvoids_;
    std::map<std::pair<Region, Region>, int> masters_;
    std::map<std::pair<Region, Region>, int> slaves_;

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
  static void ApplyAction(FieldState* field, const Action& action);

  Strategy* const strategy_;
};

#endif //SOLVER_SUPPORT_TICK_EXECUTOR_H
