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