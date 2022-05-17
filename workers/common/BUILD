package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    srcs = [
        "src/configuration.c",
        "src/connect.c",
        "src/spatialos_schema.c",
        "src/worker_log.c",
    ],
    hdrs = [
        "include/shoveler/configuration.h",
        "include/shoveler/connect.h",
        "include/shoveler/spatialos_schema.h",
        "include/shoveler/worker_log.h",
    ],
    includes = ["include"],
    deps = [
        "@shoveler//schema",
        "@thirdparty//worker_sdk",
    ],
)
