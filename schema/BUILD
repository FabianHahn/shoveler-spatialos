package(default_visibility = ["//visibility:public"])

load("//tools:schema.bzl", "generated_schema_library", "schema_bundle")

cc_binary(
    name = "schema_generator",
    srcs = ["schema_generator.c"],
    deps = [
        "//workers/client:spatialos_client_schema",
        "@shoveler//opengl",
        "@shoveler//schema",
    ],
)

generated_schema_library(
    name = "shoveler_schema",
    schema_generator = ":schema_generator",
    deps = ["@spatialos_standard_library"],
)

schema_bundle(
    name = "schema",
    srcs = [":shoveler_schema"],
)
