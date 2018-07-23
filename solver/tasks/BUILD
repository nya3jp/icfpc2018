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
    name = "line_assembler",
    srcs = ["line_assembler.cc"],
    hdrs = ["line_assembler.h"],
    deps = [
        ":command",
        "//solver/data:command",
        "//solver/support:task",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
