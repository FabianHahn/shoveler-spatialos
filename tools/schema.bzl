load("@bazel_skylib//lib:paths.bzl", "paths")

SchemaInfo = provider(
    doc = "Contains information about the schema root of a set of schema files",
    fields = {
        "schema_files": "Depset of schema files",
        "schema_paths": "Schema paths of all dependencies",
    },
)

def _schema_library_impl(ctx):
    schema_files_depset = depset(
        ctx.files.srcs,
        transitive = [dep[SchemaInfo].schema_files for dep in ctx.attr.deps],
    )

    return [
        DefaultInfo(files = schema_files_depset),
        SchemaInfo(
            schema_files = schema_files_depset,
            schema_paths = depset(
                [ctx.file.schema_path.path],
                transitive = [dep[SchemaInfo].schema_paths for dep in ctx.attr.deps],
            ),
        ),
    ]

schema_library = rule(
    implementation = _schema_library_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = True),
        "deps": attr.label_list(providers = [SchemaInfo]),
        "schema_path": attr.label(allow_single_file = True),
    },
)

def _generated_schema_library_impl(ctx):
    output_file = ctx.actions.declare_file(ctx.label.name + ".schema")
    out_dir = paths.join(ctx.bin_dir.path, ctx.label.package)

    ctx.actions.run(
        mnemonic = "SchemaGenerator",
        outputs = [output_file],
        arguments = [output_file.path],
        progress_message = "Generating schema %s" % output_file.short_path,
        executable = ctx.executable.schema_generator,
    )

    file_depset = depset(
        [output_file],
        transitive = [dep[SchemaInfo].schema_files for dep in ctx.attr.deps],
    )
    path_depset = depset(
        [out_dir],
        transitive = [dep[SchemaInfo].schema_paths for dep in ctx.attr.deps],
    )

    return [
        DefaultInfo(files = file_depset),
        SchemaInfo(
            schema_files = file_depset,
            schema_paths = path_depset,
        ),
    ]

generated_schema_library = rule(
    implementation = _generated_schema_library_impl,
    attrs = {
        "deps": attr.label_list(providers = [SchemaInfo]),
        "schema_generator": attr.label(
            executable = True,
            cfg = "exec",
            allow_files = True,
        ),
    },
)

def _schema_bundle_impl(ctx):
    output_file = ctx.actions.declare_file(ctx.label.name + ".sb")

    schema_files = depset(
        [],
        transitive = [dep[SchemaInfo].schema_files for dep in ctx.attr.srcs],
    )
    schema_paths = depset(
        [],
        transitive = [dep[SchemaInfo].schema_paths for dep in ctx.attr.srcs],
    )

    args = [
        "--bundle_out=%s" % output_file.path,
        "--load_all_schema_on_schema_path",
    ] + [
        "--schema_path=%s" % f
        for f in schema_paths.to_list()
    ] + [
        f.path
        for f in schema_files.to_list()
    ]

    # Action to call the script.
    ctx.actions.run(
        mnemonic = "SchemaCompiler",
        inputs = ctx.files.srcs,
        outputs = [output_file],
        arguments = args,
        progress_message = "Generating schema bundle %s" % output_file.short_path,
        executable = ctx.executable.schema_compiler,
    )

    return [DefaultInfo(files = depset([output_file]))]

schema_bundle = rule(
    implementation = _schema_bundle_impl,
    attrs = {
        "srcs": attr.label_list(
            providers = [SchemaInfo],
        ),
        "schema_compiler": attr.label(
            executable = True,
            cfg = "exec",
            allow_files = True,
            default = Label("@thirdparty//worker_sdk:schema_compiler"),
        ),
    },
)
