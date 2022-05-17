package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "lights",
    srcs = [
        "lights.c",
    ],
    deps = [
        "//workers/common",
    ],
)

cc_binary(
    name = "tiles",
    srcs = [
        "map.c",
        "map.h",
        "tiles.c",
        "tileset.c",
        "tileset.h",
    ],
    deps = [
        "//workers/common",
    ],
)

genrule(
    name = "tiles_snapshot",
    srcs = [
        "@shoveler_spatialos_assets//:spring.png",
        "@shoveler_spatialos_assets//:player1.png",
        "@shoveler_spatialos_assets//:player2.png",
        "@shoveler_spatialos_assets//:player3.png",
        "@shoveler_spatialos_assets//:player4.png",
    ],
    outs = ["tiles.snapshot"],
    cmd = """$(location :tiles) \
      $(location @shoveler_spatialos_assets//:spring.png) \
      7 \
      2 \
      $(location @shoveler_spatialos_assets//:player1.png) \
      $(location @shoveler_spatialos_assets//:player2.png) \
      $(location @shoveler_spatialos_assets//:player3.png) \
      $(location @shoveler_spatialos_assets//:player4.png) \
      5 \
      $@""",
    exec_tools = [":tiles"],
)

genrule(
    name = "lights_snapshot",
    srcs = [],
    outs = ["lights.snapshot"],
    cmd = "$(location :lights) $@",
    exec_tools = [":lights"],
)
