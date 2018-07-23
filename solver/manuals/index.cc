#include "solver/manuals/index.h"

#include "gflags/gflags.h"

#include "solver/manuals/example.h"
#include "solver/manuals/fa002.h"

DEFINE_string(manual, "example", "manual name");

TaskPtr MakeManualMainTask() {
  if (FLAGS_manual == "example") {
    return MakeManualExampleTask();
  }
  if (FLAGS_manual == "fa002") {
    return MakeManualFA002Task();
  }
  LOG(FATAL) << "Unknown manual name: " << FLAGS_manual;
  return nullptr;
}
