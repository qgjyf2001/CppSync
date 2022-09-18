load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "libCppSync",
    hdrs = glob(["include/*.h"]),
    deps = ["@CppHttpServer//:libThreadPool"]
)

cc_binary(
    name = "test",
    srcs = ["test.cpp"],
    linkopts = ["-lpthread"],
    copts = ["-std=c++20","-fcoroutines"],
    deps = [":libCppSync"]
)
cc_binary(
    name = "graphTest",
    srcs = ["graphTest.cpp"],
    linkopts = ["-lpthread"],
    copts = ["-std=c++20","-fcoroutines"],
    deps = [":libCppSync"]
)
