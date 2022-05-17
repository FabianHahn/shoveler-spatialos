# Bazel repository_rule which provides spatial packages as dependencies for the WORKSPACE file.

def _spatial_package_impl(repository_ctx):
    package_type = repository_ctx.attr.package_type
    name = repository_ctx.attr.package_name
    version = repository_ctx.attr.package_version

    root = repository_ctx.path(".")
    directory = str(root)
    spatial_cmd = """
    spatial package get \
      --log_level warning \
      --no_animation \
      --worker_package_timeout_s 60m \
      --force \
      --unzip \
      {package_type} \
      {name} \
      {version} \
      {directory}
    """.format(
        package_type = package_type,
        name = name,
        version = version,
        directory = directory,
    )

    bash_exe = repository_ctx.os.environ["BAZEL_SH"] if "BAZEL_SH" in repository_ctx.os.environ else "bash"
    result = repository_ctx.execute([bash_exe, "-c", spatial_cmd], quiet = False)
    if result.return_code:
        fail("Couldn't retrieve spatial package [type=%s, name=%s, version=%s]:\n%s\n%s" %
             (package_type, name, version, result.stdout, result.stderr))

    result = repository_ctx.execute([bash_exe, "-c", "/usr/bin/find . -type f"])
    if result.return_code:
        fail("Couldn't find spatial package contents:\n%s\n%s" % (result.stdout, result.stderr))

    files = result.stdout.split("\n")
    if repository_ctx.attr.build_file:
        repository_ctx.symlink(repository_ctx.path(repository_ctx.attr.build_file), "BUILD")
    else:
        repository_ctx.file("BUILD", "exports_files([\n    %s\n])" %
                                     ",\n    ".join(["\"%s\"" % f[2:] for f in files if f]))

_RULE_ATTRS = {
    "package_type": attr.string(mandatory = True),
    "package_name": attr.string(mandatory = True),
    "package_version": attr.string(mandatory = True),
    "build_file": attr.label(mandatory = False),
}

spatial_package = repository_rule(
    implementation = _spatial_package_impl,
    attrs = _RULE_ATTRS,
)
