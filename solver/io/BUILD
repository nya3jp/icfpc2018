package(default_visibility=["//visibility:public"])

cc_library(
    name = "model_reader",
    srcs = ["model_reader.cc"],
    hdrs = ["model_reader.h"],
    deps = [
        "//solver/data:matrix",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "trace_writer",
    hdrs = ["trace_writer.h"],
    deps = [
        "//solver/data:command",
        "//solver/data:geometry",
    ],
)

cc_library(
    name = "trace_writer_impl",
    srcs = ["trace_writer_impl.cc"],
    hdrs = ["trace_writer_impl.h"],
    deps = [
        ":trace_writer",
        "@com_github_google_glog//:glog",
    ],
)
