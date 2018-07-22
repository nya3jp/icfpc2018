cc_binary(
    name = "sim",
    srcs = ["sim.cc"],
    deps = [
        "//solver/data:command",
        "//solver/data:geometry",
        "//solver/data:matrix",
        "//solver/io:model_reader",
        "//solver/io:trace_writer",
        "//solver/support:tick_executor",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
