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
        "//solver/data:action",
        "//solver/data:command",
        "//solver/data:geometry",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "task_executor",
    srcs = ["task_executor.cc"],
    hdrs = ["task_executor.h"],
    deps = [
        ":task",
        ":tick_executor",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "task",
    srcs = ["task.cc"],
    hdrs = ["task.h"],
    deps = [
        "//solver/support:tick_executor",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "union_find",
    srcs = ["union_find.cc"],
    hdrs = ["union_find.h"],
    deps = [
        "//solver/data:geometry",
        "@com_github_google_glog//:glog",
    ],
)
