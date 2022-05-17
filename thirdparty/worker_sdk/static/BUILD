package(default_visibility = ["//visibility:public"])

cc_library(
    name = "static",
    srcs = select({
        "@//:windows": [
            "improbable_worker.lib",
            "ssl.lib",
        ],
        "//conditions:default": [
            "libimprobable_worker.a",
            "libssl.a",
        ],
    }),
    licenses = ["permissive"],
    linkopts = select({
        "@//:linux": [
            "-ldl",
            "-lm",
            "-lpthread",
        ],
        "@//:windows": [
            "-DEFAULTLIB:advapi32.lib",
            "-DEFAULTLIB:gdi32.lib",
            "-DEFAULTLIB:user32.lib",
            "-DEFAULTLIB:ws2_32.lib",
        ],
        "//conditions:default": [],
    }),
    deps = [
        "@zlib",
    ],
)
