#include "solver/manuals/index.h"

#include "gflags/gflags.h"

#include "solver/manuals/example.h"

DEFINE_string(manual, "example", "manual name");

TaskPtr MakeManualMainTask() {
  if (FLAGS_manual == "example") {
    return MakeManualExampleTask();
  }
  LOG(FATAL) << "Unknown manual name: " << FLAGS_manual;
  return nullptr;
}
