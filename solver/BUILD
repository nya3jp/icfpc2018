cc_binary(
    name = "naive",
    srcs = ["naive.cc"],
    deps = [
        ":geometry",
        ":model",
        ":trace_writer",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "model",
    srcs = ["model.cc"],
    hdrs = ["model.h"],
    deps = [
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "trace_writer",
    srcs = ["trace_writer.cc"],
    hdrs = ["trace_writer.h"],
    deps = [
        ":geometry",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "geometry",
    hdrs = ["geometry.h"],
)
