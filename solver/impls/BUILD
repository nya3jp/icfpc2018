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
        "//solver/data:model",
        "//solver/io:trace_writer",
        "//solver/support:careless_controller",
        "@com_github_gflags_gflags//:gflags",
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
        "//solver/data:model",
        "//solver/io:trace_writer",
        "//solver/support:careless_controller",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
