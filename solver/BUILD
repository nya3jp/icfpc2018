cc_binary(
    name = "solver",
    srcs = ["solver.cc"],
    deps = [
        "//solver/data:geometry",
        "//solver/data:matrix",
        "//solver/io:model_reader",
        "//solver/io:trace_writer",
        "//solver/impls:base",
        "//solver/impls:bbgvoid",
        "//solver/impls:task_executor_example",
        "//solver/impls:naive",
        "//solver/impls:tick_executor_example",
        "//solver/support:careless_controller",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
