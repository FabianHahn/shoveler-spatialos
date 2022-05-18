load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    sha256 = "1c531376ac7e5a180e0237938a2536de0c54d93f5c278634818e0efc952dd56c",
    urls = [
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
    ],
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

load("@bazel_skylib//lib:versions.bzl", "versions")

versions.check(
    maximum_bazel_version = "5.1.1",
    minimum_bazel_version = "5.1.1",
)

# We make shoveler/ its own bazel workspace because it allows to run `bazel build ...` without
# having all targets defined in third-party BUILD files in this directory buildable.
local_repository(
    name = "shoveler_vendored",
    path = "shoveler",
)

# Since we inherit all of shoveler's dependencies, we also steal its thirdparty local workspace.
http_archive(
    name = "shoveler_thirdparty",
    sha256 = "b3c10c4658e7f5db77ebeaf10aeb20f2d1796bacc737751d58a1352d4ff80791",
    strip_prefix = "shoveler-8d0d5202e4654ada69a44c9827e4bcf6cebade47/thirdparty",
    url = "https://github.com/FabianHahn/shoveler/archive/8d0d5202e4654ada69a44c9827e4bcf6cebade47.tar.gz",
)

# We make thirdparty/ its own bazel workspace because it allows to run `bazel build ...` without
# having all targets defined in third-party BUILD files in this directory buildable.
local_repository(
    name = "thirdparty",
    path = "thirdparty",
)

http_archive(
    name = "fakeglib",
    sha256 = "9e28f304c01493f13d46027330b04f3936bf3ca2c47815664cd8d9b5d113584c",
    strip_prefix = "fakeglib-823da8d10e96d62286fb538b70863366fd43b505",
    url = "https://github.com/FabianHahn/fakeglib/archive/823da8d10e96d62286fb538b70863366fd43b505.tar.gz",
)

http_archive(
    name = "freetype",
    build_file = "@shoveler_thirdparty//freetype:BUILD",
    sha256 = "11b13952b78e55f399a5f185c9c46e3bec0da3989066ce76c984d446a8ef7090",
    strip_prefix = "freetype-VER-2-12-0",
    url = "https://github.com/freetype/freetype/archive/VER-2-12-0.tar.gz",
)

http_archive(
    name = "glfw",
    build_file = "@shoveler_thirdparty//glfw:BUILD",
    sha256 = "fd21a5f65bcc0fc3c76e0f8865776e852de09ef6fbc3620e09ce96d2b2807e04",
    strip_prefix = "glfw-3.3.7",
    url = "https://github.com/glfw/glfw/archive/3.3.7.tar.gz",
)

http_archive(
    name = "shoveler",
    repo_mapping = {
        "@thirdparty": "@shoveler_thirdparty",
    },
    sha256 = "b3c10c4658e7f5db77ebeaf10aeb20f2d1796bacc737751d58a1352d4ff80791",
    strip_prefix = "shoveler-8d0d5202e4654ada69a44c9827e4bcf6cebade47",
    url = "https://github.com/FabianHahn/shoveler/archive/8d0d5202e4654ada69a44c9827e4bcf6cebade47.tar.gz",
)

http_archive(
    name = "shoveler_spatialos_assets",
    build_file = "@thirdparty//assets:BUILD",
    sha256 = "ebfef10c363c01612ec2340c68e6854af4c98bbcd515451ee5662dcd2b8dbb6a",
    strip_prefix = "shoveler-spatialos-assets-f0ad55c0e81403de65f5c306b8fb1e79b74d7d3a",
    url = "https://github.com/FabianHahn/shoveler-spatialos-assets/archive/f0ad55c0e81403de65f5c306b8fb1e79b74d7d3a.tar.gz",
)

http_archive(
    name = "libpng",
    build_file = "@shoveler_thirdparty//libpng:BUILD",
    sha256 = "ca74a0dace179a8422187671aee97dd3892b53e168627145271cad5b5ac81307",
    strip_prefix = "libpng-1.6.37",
    url = "https://github.com/glennrp/libpng/archive/v1.6.37.tar.gz",
)

http_archive(
    name = "zlib",
    build_file = "@shoveler_thirdparty//zlib:BUILD",
    sha256 = "d8688496ea40fb61787500e863cc63c9afcbc524468cedeb478068924eb54932",
    strip_prefix = "zlib-1.2.12",
    url = "https://github.com/madler/zlib/archive/v1.2.12.tar.gz",
)

load("//tools:spatial_package.bzl", "spatial_package")

SPATIALOS_SDK_VERSION = "16.1.0"

spatial_package(
    name = "spatialos_worker_sdk_c_headers",
    package_name = "c_headers",
    build_file = "@thirdparty//worker_sdk/headers:BUILD",
    package_type = "worker_sdk",
    package_version = SPATIALOS_SDK_VERSION,
)

spatial_package(
    name = "spatialos_worker_sdk_linux",
    package_name = "c-static-x86_64-clang1000_pic-linux",
    build_file = "@thirdparty//worker_sdk/static:BUILD",
    package_type = "worker_sdk",
    package_version = SPATIALOS_SDK_VERSION,
)

spatial_package(
    name = "spatialos_worker_sdk_macos",
    package_name = "c-static-x86_64-clang-macos",
    build_file = "@thirdparty//worker_sdk/static:BUILD",
    package_type = "worker_sdk",
    package_version = SPATIALOS_SDK_VERSION,
)

spatial_package(
    name = "spatialos_worker_sdk_windows",
    package_name = "c-static-x86_64-vc141_md-win32",
    build_file = "@thirdparty//worker_sdk/static:BUILD",
    package_type = "worker_sdk",
    package_version = SPATIALOS_SDK_VERSION,
)

spatial_package(
    name = "spatialos_standard_library",
    package_name = "standard_library",
    build_file = "@thirdparty//worker_sdk/schema:BUILD",
    package_type = "schema",
    package_version = SPATIALOS_SDK_VERSION,
)

spatial_package(
    name = "spatialos_schema_compiler_linux",
    package_name = "schema_compiler-x86_64-linux",
    package_type = "tools",
    package_version = SPATIALOS_SDK_VERSION,
)

spatial_package(
    name = "spatialos_schema_compiler_macos",
    package_name = "schema_compiler-x86_64-macos",
    package_type = "tools",
    package_version = SPATIALOS_SDK_VERSION,
)

spatial_package(
    name = "spatialos_schema_compiler_windows",
    package_name = "schema_compiler-x86_64-win32",
    package_type = "tools",
    package_version = SPATIALOS_SDK_VERSION,
)

SPATIALOS_RUNTIME_VERSION = "16.1.1"

spatial_package(
    name = "spatialos_runtime_linux",
    package_name = "x86_64-linux",
    package_type = "runtime",
    package_version = SPATIALOS_RUNTIME_VERSION,
)

spatial_package(
    name = "spatialos_runtime_macos",
    package_name = "x86_64-macos",
    package_type = "runtime",
    package_version = SPATIALOS_RUNTIME_VERSION,
)

spatial_package(
    name = "spatialos_runtime_windows",
    package_name = "x86_64-win32",
    package_type = "runtime",
    package_version = SPATIALOS_RUNTIME_VERSION,
)
