package(default_visibility = ["//visibility:public"])

cc_library(
    name = "spatialos_client_schema",
    srcs = [
        "spatialos_client_schema.c",
        "spatialos_client_schema.h",
    ],
    deps = [
        "//workers/common",
        "@shoveler//schema",
    ],
)

cc_binary(
    name = "client",
    srcs = [
        "client.c",
        "configuration.c",
        "configuration.h",
        "interest.c",
        "interest.h",
    ],
    deps = [
        ":spatialos_client_schema",
        "//workers/common",
        "@shoveler//client",
    ],
)
