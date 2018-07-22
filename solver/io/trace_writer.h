#ifndef SOLVER_IO_TRACE_WRITER_H
#define SOLVER_IO_TRACE_WRITER_H

#include "solver/data/command.h"

class TraceWriter {
 public:
  virtual ~TraceWriter() = default;
  TraceWriter(const TraceWriter& other) = delete;

  virtual void Command(const Command& command) = 0;

 protected:
  TraceWriter() = default;
};

#endif // SOLVER_IO_TRACE_WRITER_H
