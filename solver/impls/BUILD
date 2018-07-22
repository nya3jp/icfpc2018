package(default_visibility=["//visibility:public"])

cc_library(
    name = "base",
    hdrs = ["base.h"],
)

cc_library(
    name = "naive",
    srcs = ["naive.cc"],
    hdrs = ["naive.h"],
    deps = [
        ":base",
        "//solver/data:geometry",
        "//solver/data:matrix",
        "//solver/io:trace_writer",
        "//solver/support:careless_controller",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "bbgvoid",
    srcs = ["bbgvoid.cc"],
    hdrs = ["bbgvoid.h"],
    deps = [
        ":base",
        "//solver/data:geometry",
        "//solver/data:matrix",
        "//solver/io:trace_writer",
        "//solver/support:careless_controller",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "tick_executor_example",
    srcs = ["tick_executor_example.cc"],
    hdrs = ["tick_executor_example.h"],
    deps = [
        ":base",
        "//solver/data:matrix",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "//solver/support:tick_executor",
        "@com_github_google_glog//:glog",
    ],
)
