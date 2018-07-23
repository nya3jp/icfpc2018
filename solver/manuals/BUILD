package(default_visibility=["//visibility:public"])

cc_library(
    name = "manuals",
    srcs = [
        "debug.cc",
        "example.cc",
        "fa002.cc",
        "index.cc",
    ],
    hdrs = [
        "debug.h",
        "example.h",
        "fa002.h",
        "index.h",
    ],
    deps = [
        "//solver/support:task",
        "//solver/tasks:manual_assembler",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
