package(default_visibility = ["//visibility:public"])

load("//tools:deployment.bzl", "local_deployment")

config_setting(
    name = "linux",
    constraint_values = ["@bazel_tools//platforms:linux"],
)

config_setting(
    name = "macos",
    constraint_values = ["@bazel_tools//platforms:osx"],
)

config_setting(
    name = "windows",
    constraint_values = ["@bazel_tools//platforms:windows"],
)

config_setting(
    name = "windows_debug",
    constraint_values = ["@bazel_tools//platforms:windows"],
    values = {
        "compilation_mode": "dbg",
    },
)

config_setting(
    name = "windows_release",
    constraint_values = ["@bazel_tools//platforms:windows"],
    values = {
        "compilation_mode": "opt",
    },
)

local_deployment(
    name = "tiles",
    launch_config = "tiles_standalone.json",
    schema_bundle = "//schema",
    snapshot = "//seeders:tiles.snapshot",
)

local_deployment(
    name = "lights",
    launch_config = "lights_standalone.json",
    schema_bundle = "//schema",
    snapshot = "//seeders:lights.snapshot",
)
