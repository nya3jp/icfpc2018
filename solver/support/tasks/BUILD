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
