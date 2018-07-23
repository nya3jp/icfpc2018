package(default_visibility=["//visibility:public"])

cc_library(
    name = "command",
    srcs = ["command.cc"],
    hdrs = ["command.h"],
    deps = [
        "//solver/data:command",
        "//solver/support:task",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "plane_assembler",
    srcs = ["plane_assembler.cc"],
    hdrs = ["plane_assembler.h"],
    deps = [
        ":command",
        "//solver/data:command",
        "//solver/support:task",
        "@com_github_google_glog//:glog",
    ],
)
