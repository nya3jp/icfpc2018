package(default_visibility=["//visibility:public"])

cc_library(
    name = "careless_controller",
    srcs = ["careless_controller.cc"],
    hdrs = ["careless_controller.h"],
    deps = [
        "//solver/data:geometry",
        "//solver/io:trace_writer",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "tick_executor",
    srcs = ["tick_executor.cc"],
    hdrs = ["tick_executor.h"],
    deps = [
        "//solver/data:command",
        "//solver/data:geometry",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "action_executor",
    srcs = ["action_executor.cc"],
    hdrs = ["action_executor.h"],
    deps = [
        ":tick_executor",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "//solver/support/actions:base",
        "@com_github_google_glog//:glog",
    ],
)