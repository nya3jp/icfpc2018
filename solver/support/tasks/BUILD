package(default_visibility=["//visibility:public"])

cc_library(
    name = "base",
    srcs = ["base.cc"],
    hdrs = ["base.h"],
    deps = [
        "//solver/data:state",
        "//solver/support:tick_executor",
    ],
)

cc_library(
    name = "mock",
    hdrs = ["mock.h"],
    deps = [
        ":base",
    ]
)

cc_library(
    name = "sequence",
    srcs = ["sequence.cc"],
    hdrs = ["sequence.h"],
    deps = [
        ":base",
        "@com_github_google_glog//:glog",
    ]
)
