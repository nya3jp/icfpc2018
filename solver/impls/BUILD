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
    name = "bbgvoid_task",
    srcs = ["bbgvoid_task.cc"],
    hdrs = ["bbgvoid_task.h"],
    deps = [
        ":base",
        "//solver/data:geometry",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "//solver/support:task",
        "//solver/support:task_executor",
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

cc_library(
    name = "task_executor_example",
    srcs = ["task_executor_example.cc"],
    hdrs = ["task_executor_example.h"],
    deps = [
        ":base",
        "//solver/data:matrix",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "//solver/support:task",
        "//solver/support:task_executor",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "fission_naive",
    srcs = ["fission_naive.cc"],
    hdrs = ["fission_naive.h"],
    deps = [
        ":base",
        "//solver/data:matrix",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "//solver/support:task",
        "//solver/support:task_executor",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "generic_task",
    srcs = ["generic_task.cc"],
    hdrs = ["generic_task.h"],
    deps = [
        ":base",
        "//solver/data:matrix",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "//solver/support:task",
        "//solver/support:task_executor",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "delete_strategy",
    srcs = ["deleter2.cc"],
    hdrs = ["deleter.h"],
    deps = [
        ":base",
        "//solver/data:matrix",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "//solver/support:tick_executor",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "delete_strategy3",
    srcs = ["deleter3.cc"],
    hdrs = ["deleter3.h"],
    deps = [
        ":base",
        "//solver/data:matrix",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "//solver/support:tick_executor",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "reassemble_naive",
    srcs = ["reassemble_naive.cc"],
    hdrs = ["reassemble_naive.h"],
    deps = [
        ":base",
        ":bbgvoid_task",
        ":delete_strategy",
        ":delete_strategy3",
        ":fission_naive",
        ":generic_task",
        ":naive",
        "//solver/data:matrix",
        "//solver/io:trace_writer",
        "//solver/tasks:line_assembler",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "fa",
    srcs = ["fa.cc"],
    hdrs = ["fa.h"],
    deps = [
        ":base",
        "//solver/data:matrix",
        "//solver/data:state",
        "//solver/io:trace_writer",
        "//solver/support:tick_executor",
        "@com_github_google_glog//:glog",
    ],
)
