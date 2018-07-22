package(default_visibility=["//visibility:public"])

cc_library(
    name = "matrix",
    srcs = ["matrix.cc"],
    hdrs = ["matrix.h"],
    deps = [
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "geometry",
    srcs = ["geometry.cc"],
    hdrs = ["geometry.h"],
    deps = [
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "state",
    srcs = ["state.cc"],
    hdrs = ["state.h"],
    deps = [
        ":geometry",
        ":matrix",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "command",
    hdrs = ["command.h"],
    deps = [
        ":geometry",
    ],
)