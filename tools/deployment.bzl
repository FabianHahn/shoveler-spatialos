def _local_deployment_impl(ctx):
    script = ctx.actions.declare_file("%s_runner" % ctx.label.name)
    script_content = """#!/bin/bash
    {runtime} \
      --config {config} \
      --snapshot {snapshot} \
      --schema-bundle {schema_bundle}
    """.format(
        runtime = ctx.executable.runtime.short_path,
        config = ctx.file.launch_config.short_path,
        snapshot = ctx.file.snapshot.short_path,
        schema_bundle = ctx.file.schema_bundle.short_path,
    )
    ctx.actions.write(script, script_content, is_executable = True)

    runfiles = ctx.runfiles(files = [
        ctx.executable.runtime,
        ctx.file.launch_config,
        ctx.file.snapshot,
        ctx.file.schema_bundle,
    ])
    return [DefaultInfo(executable = script, runfiles = runfiles)]

local_deployment = rule(
    implementation = _local_deployment_impl,
    executable = True,
    attrs = {
        "launch_config": attr.label(allow_single_file = True),
        "snapshot": attr.label(allow_single_file = True),
        "schema_bundle": attr.label(allow_single_file = True),
        "runtime": attr.label(
            executable = True,
            cfg = "exec",
            allow_single_file = True,
            default = Label("@thirdparty//runtime"),
        ),
    },
)
