cc_binary(
    name = "solver",
    srcs = ["solver.cc"],
    deps = [
        "//solver/data:geometry",
        "//solver/data:model",
        "//solver/io:trace_writer",
        "//solver/impls:base",
        "//solver/impls:naive",
        "//solver/impls:bbgvoid",
        "//solver/support:careless_controller",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
