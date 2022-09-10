load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "libThreadPool",
    srcs = ["threadPool/threadPool.cpp"],
    includes = ["threadPool"],
    hdrs = glob(["threadPool/*.h"]),
    copts = ["-std=c++20","-fcoroutines"]
)
cc_binary(
    name = "test",
    srcs = glob(["include/*.h"])+["test.cpp"],
    linkopts = ["-lpthread"],
    copts = ["-std=c++20","-fcoroutines"],
    deps = [":libThreadPool"],
)
cc_binary(
    name = "graphTest",
    srcs = glob(["include/*.h"])+["graphTest.cpp"],
    linkopts = ["-lpthread"],
    copts = ["-std=c++20","-fcoroutines"],
    deps = [":libThreadPool"],
)