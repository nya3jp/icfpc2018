package(default_visibility = ["//visibility:public"])

cc_toolchain_suite(
	name = "custom_toolchain",
	toolchains = {
		"darwin|compiler": ":cc-compiler-darwin_x86_64",
	}
)

cc_library(
    name = "malloc",
)

cc_library(
    name = "stl",
)

filegroup(
    name = "empty",
    srcs = [],
)

filegroup(
    name = "cc_wrapper",
    srcs = ["cc_wrapper.sh"],
)

filegroup(
    name = "osx_tools_darwin_x86_64",
    srcs = [
        ":cc_wrapper",
        ":libtool",
        ":make_hashed_objlist.py",
        ":wrapped_clang",
        ":wrapped_clang_pp",
        ":wrapped_ar",
        ":xcrunwrapper.sh",
    ],
)

cc_toolchain(
    name = "cc-compiler-darwin_x86_64",
    all_files = ":osx_tools_darwin_x86_64",
    compiler_files = ":osx_tools_darwin_x86_64",
    cpu = "darwin",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":osx_tools_darwin_x86_64",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":osx_tools_darwin_x86_64",
    supports_param_files = 0,
)
