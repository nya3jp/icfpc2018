package(default_visibility=["//visibility:public"])

cc_library(
    name = "manuals",
    srcs = [
        "example.cc",
        "index.cc",
    ],
    hdrs = [
        "example.h",
        "index.h",
    ],
    deps = [
        "//solver/support:task",
        "//solver/tasks:manual_assembler",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
