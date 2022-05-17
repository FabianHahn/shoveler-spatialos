package(default_visibility = ["//visibility:public"])

alias(
    name = "runtime",
    actual = select({
        "@//:linux": "@spatialos_runtime_linux//:runtime",
        "@//:macos": "@spatialos_runtime_macos//:runtime",
        "@//:windows": "@spatialos_runtime_windows//:runtime.exe",
    }),
)
