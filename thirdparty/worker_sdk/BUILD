package(default_visibility = ["//visibility:public"])

cc_library(
    name = "worker_sdk",
    licenses = ["permissive"],
    deps = select({
        "@//:linux": ["@spatialos_worker_sdk_linux//:static"],
        "@//:macos": ["@spatialos_worker_sdk_macos//:static"],
        "@//:windows_debug": ["@spatialos_worker_sdk_windows_debug//:static"],
        "@//:windows_release": ["@spatialos_worker_sdk_windows_release//:static"],
    }) + [
        "@spatialos_worker_sdk_c_headers//:headers",
    ],
)

alias(
    name = "schema_compiler",
    actual = select({
        "@//:linux": "@spatialos_schema_compiler_linux//:schema_compiler",
        "@//:macos": "@spatialos_schema_compiler_macos//:schema_compiler",
        "@//:windows": "@spatialos_schema_compiler_windows//:schema_compiler.exe",
    }),
)
