# shoveler-spatialos

## Introduction

This repository contains an example integration of the [shoveler](https://github.com/FabianHahn/shoveler) game engine into [Improbable's SpatialOS](https://improbable.io/spatialos). SpatialOS is primarily used together with its dedicated integrations for popular game engines such as [Unity](https://unity3d.com/) and [Unreal Engine](https://www.unrealengine.com), but it also includes lower-level language SDKs for [C](https://docs.improbable.io/reference/14.1/csdk/introduction), [C++](https://docs.improbable.io/reference/14.1/cppsdk/introduction), [C#](https://docs.improbable.io/reference/14.1/csharpsdk/introduction) and [Java](https://docs.improbable.io/reference/14.1/javasdk/introduction) which can be used to [integrate any game engine](https://docs.improbable.io/reference/14.1/shared/byoe/introduction).
This project uses the C++ SDK to run _shoveler_ workers on both the client and the server side to implement a simple interactive 3D sandbox demo game, which harnesses the power of SpatialOS to enable multiplayer in the cloud.
Though I work at Improbable, this is a personal project built in my free time using publicly available resources.
It is not an Improbable product.

There are currently two playable demos, **boxes** and **tiles**.
Since this integration defines the whole game world within the SpatialOS simulation, both demos can be played with the exact same client worker binary.

### Lights demo

![lights demo screenshot](https://github.com/FabianHahn/shoveler-spatialos-assets/raw/master/lights.png)

Client workers render a 3D world with moving light sources and simple geometry such as cubes and planes, while the server worker does basic client lifecycle management and object spawning on behalf of clients.
Each player is a small point light with a random color that can fly around freely with WASD + mouse controls, and spawn cubes of their color by clicking the right mouse button.
Players can see each other flying around and placing cubes, and everything in the world is fully persistent even if players disconnect.

### Tiles demo

![tiles demo screenshot](https://github.com/FabianHahn/shoveler-spatialos-assets/raw/master/tiles.png)

Client workers render a 2D world based on [tile mapping](https://developer.mozilla.org/en-US/docs/Games/Techniques/Tilemaps), while the server worker does basic client lifecycle management and world interaction on behalf of clients.
Each player is represented by a sphere that can move around in the world with arrow key controls but collides with objects, and can dig a hole in the ground at their current position by clicking the right mouse button.
World data is dynamically streamed in and out as players move around or press the W and S keys to zoom, which will adjust their interest to always keep the screen filled. 
Players can see each other moving around and digging holes, and everything in the world is fully persistent even if players disconnect.

The tileset assets used in this demo were kindly provided to me by [Charles Micou](https://github.com/CharlesMicou).

## Versions

If you've just navigated to this repository on GitHub for the first time and are looking to get started, I recommend you select one of the **stable version tags** from the table below instead of using the [`master`](https://github.com/FabianHahn/shoveler-spatialos/tree/master) branch. While I try to keep [`master`](https://github.com/FabianHahn/shoveler-spatialos/tree/master) as stable as possible, most active development happens there and I do accidentally break things from time to time.

| Version | SpatialOS SDK | Project Structure | Release Notes |
| --- | --- | --- | --- |
| [`master`](https://github.com/FabianHahn/shoveler-spatialos/tree/master) | C++ (version 14.1.0) | [FPL beta](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/introduction) | n/a (active development) |
| [`0.3`](https://github.com/FabianHahn/shoveler-spatialos/tree/v0.3) | C++ (version 14.1.0) | [FPL beta](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/introduction) | full query-based interest support |
| [`0.2`](https://github.com/FabianHahn/shoveler-spatialos/tree/v0.2) | C++ (version 13.7.1) | [FPL beta](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/introduction) | FPL port, working tiles demo |
| [`0.1`](https://github.com/FabianHahn/shoveler-spatialos/tree/v0.1) | C++ (version 13.5.1) | [SPL](https://docs.improbable.io/reference/14.1/shared/project-layout/files-and-directories) | Initial release, working lights demo |

## Repository structure

The overall directory structure of this repository was chosen to comply with the [SpatialOS beta flexible project structure](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/introduction) while still integrating nicely with the [CMake](https://cmake.org/) build system. The most important files and directories in the repository are:
 * [`assets/`](assets/): Asset references fetched using a [CMake `ExternalData` rule](https://cmake.org/cmake/help/v3.3/module/ExternalData.html) from a [separate asset repository](https://github.com/FabianHahn/shoveler-spatialos-assets) to avoid binary files in this repository.
 * [`schema/shoveler.schema`](schema/shoveler.schema): [SpatialOS schema](https://docs.improbable.io/reference/14.1/shared/schema/introduction#schema-introduction) for the project.
 * [`seeders/lights.cpp`](seeders/lights.cpp): Complete seeder tool source code for initial snapshot generation of the lights demo.
 * [`seeders/tiles/map.cpp`](seeders/tiles/map.cpp): Map generation source code for tiles demo.
 * [`seeders/tiles.cpp`](seeders/tiles.cpp): Seeder tool source code entrypoint for initial snapshot generation of the tiles demo.
 * [`shoveler/`](shoveler/): Unchanged git subtree of the [shoveler](https://github.com/FabianHahn/shoveler) repository.
 * [`worker_sdk/CMakeLists.txt`](worker_sdk/CMakeLists.txt): Helper CMake file to [link against the SpatialOS C++ worker SDK](https://docs.improbable.io/reference/14.1/cppsdk/building).
 * [`workers/client/`](workers/client/): Complete source code for client worker.
 * [`workers/client/client.cpp`](workers/client/client.cpp): Source file containing main function for client worker.
 * [`workers/client/worker.json`](workers/client/worker.json): [SpatialOS client worker configuration file](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/reference/client-worker-configuration)
 * [`workers/server/server.cpp`](workers/server/server.cpp): Complete source code for server worker
 * [`workers/server/worker.json`](workers/server/worker.json): [SpatialOS server worker configuration file](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/reference/server-worker-configuration)
  * [`workers/updater/`](workers/updater/): Complete source code for runtime asset updater worker.
 * [`CMakeLists.txt`](CMakeLists.txt): Root CMake project definition file.
 * [`CMakeLists.txt.external.in`](CMakeLists.txt.external.in): Helper CMake file to [download SpatialOS dependencies](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/build-process/worker-build-process#1-download-dependencies).
 * [`lights.json`](lights.json): [SpatialOS launch configuration file](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/reference/launch-configuration) for the lights demo.
 * [`spatialos.json`](spatialos.json): [SpatialOS project configuration file](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/reference/project-configuration) containing the project name and referencing the other configuration files.
 * [`tiles.json`](tiles.json): [SpatialOS launch configuration file](https://docs.improbable.io/reference/14.1/shared/flexible-project-layout/reference/launch-configuration) for the tiles demo.

## Usage

**shoveler-spatialos** was tested to compile out of the box on Linux (tested with _gcc_ and _clang_) and Windows (tested with _Visual Studio 2015_). It might work on other platforms, but since I do not have access to them I am unable to support them and guarantee that they work.

### Prerequisites

You need to have [CMake](https://cmake.org/) 3.13 or later and the [`spatial` CLI](https://docs.improbable.io/reference/14.1/shared/spatialos-cli-introduction) both installed and available in your system `PATH`. You also need a bash-compatible shell on Windows, which you should have through your [Git](https://git-scm.com/) installation that you will also need to clone this repository.

On Linux, you further need headers for the [X Window System](http://www.opengroup.org/tech/desktop/x-window-system/). If you are using a Linux distribution based on the [APT](https://wiki.debian.org/Apt) package manager such as _Debian_ or _Ubuntu_, you can install them with the following command:
```
sudo apt-get install xorg-dev libxi-dev
```

You also need a SpatialOS account that you can create for free [here](https://improbable.io/get-spatialos).

To run clients, your machine needs a graphics card with a driver supporting OpenGL 4.5 or later.

### Build

Clone the repository and switch into it:
```
git clone https://github.com/FabianHahn/shoveler-spatialos
cd shoveler-spatialos
```

Optionally, you might want to check out the latest stable tag:
```
git checkout v0.3
```

Create a CMake build directory and switch into it:
```
mkdir build
cd build
```

Run CMake to configure the project, which includes downloading the correct version of the SpatialOS C++ SDK, the schema compiler, and the standard library schema:
```
# Linux:
cmake ..

# Windows:
cmake -G "Visual Studio 14 2015 Win64" ..
```

The following command will build a complete project assembly by generating C++ code for the included [schema](schema/shoveler.schema), compiling shoveler and its dependencies, compiling the [seeders](workers/seeders) and running them to generate initial snapshots, and building executables for both the included [client](workers/client/client.cpp) and [server](workers/server/server.cpp) workers:
```
# Linux (change N to the number of threads you want to compile with):
make -jN

# Windows:
cmake --build . --config release
```

### Local deployment

To run a local deployment, simply run the following command:
```
spatial alpha local launch --launch_config=lights.json --snapshot=build/seeders/lights.snapshot # start the lights demo
spatial alpha local launch --launch_config=tiles.json --snapshot=build/seeders/tiles.snapshot # start the tiles demo
```

Once the SpatialOS Runtime has started up, you should be able to open the [Inspector](http://localhost:21000/inspector) in your browser and see the entities present in the seed snapshot, as well as a connected managed server worker.

To connect a client, switch to the right directory and simply run it with the correct arguments pointing to your local deployment:
```
# Linux:
cd build/workers/client
./ShovelerClient # launch with random worker ID
./ShovelerClient ShovelerClient1 localhost 7777 # launch with specific worker ID

# Windows:
cd build/workers/client/Release
ShovelerClient.exe # launch with random worker ID
ShovelerClient.exe ShovelerClient1 localhost 7777 # launch with specific worker ID
```

When running locally, you can either specify zero or three arguments. In the latter case, the first argument to the `ShovelerClient` executable is the worker ID to use, the second one is the hostname to connect to, and the third one is the receptionist port of the SpatialOS Runtime.

You can connect multiple clients to the same local deployment as long as you choose a fresh worker ID that hasn't been used before since the deployment was started.

### Cloud deployment

While it is perfectly fine to connect to cloud deployments from Windows, SpatialOS currently only runs server workers built for Linux. If you start a cloud deployment from a Windows assembly, it will report errors trying to start server workers, and clients won't be able to complete their login.

Start by editing the [`spatialos.json`](spatialos.json) configuration file and replace the value of the `"projectName"` entry with your SpatialOS project name in quotes. You can see your project name by visiting the [SpatialOS console](https://console.improbable.io/projects) page.

Next, you need to upload your built assembly. You can choose any assembly name you like, the following is just an example:
```
spatial alpha cloud upload -a spatialos-shoveler-assembly
```

To launch a cloud deployment, run the following command from the root directory of the project. You have to specify the same assembly name as before, and further an arbitrary deployment name, which in this example is simply `shoveler_spatialos`:
```
spatial alpha cloud launch -a spatialos-shoveler-assembly -d shoveler_spatialos --launch_config=lights.json --snapshot=build/seeders/lights.snapshot # launch lights demo
spatial alpha cloud launch -a spatialos-shoveler-assembly -d shoveler_spatialos --launch_config=tiles.json --snapshot=build/seeders/tiles.snapshot # launch tiles demo
```

It will take a few minutes for your deployment to start up, which you can also monitor in the [SpatialOS console](https://console.improbable.io/projects). As soon as it is running, open the deployment overview page in the console and click on the "LAUNCH" button on the left side. This will open a dialog instructing you to install the SpatialOS Launcher. The Launcher currently only supports clients built with _Unity_ or _Unreal Engine_ and thus won't work to start client workers for this project, so ignore Step 1 and instead copy the link that the blue "Launch" button in Step 2 points to (e.g. in Chrome: right click, select "copy link address"). Then connect a client by simply passing this link as its only command line argument in quotes:
```
# Linux:
cd build/workers/client
./ShovelerClient "spatialos.launch:project_name-shoveler_spatialos?token=ey..."

# Windows:
cd build/workers/client/Release
ShovelerClient.exe "spatialos.launch:project_name-shoveler_spatialos?token=ey..."
```

You can connect any number of clients from any number of different machines using the same login token. Since the worker executables are statically linked, you can distribute them directly without including any other files from the repository or the build.

### Input bindings

When running a client worker, click into the window's drawing area to enable mouse and keyboard input for that worker.
You can detach input capture again by pressing the Alt + Tab key combination and switch out to a different window.
While input is captured, the following bindings are available:
 - Mouse movement: Control the camera tilt in 3D mode.
 - Right mouse button: Interact with the world, currently by spawning a cube in the lights demo or digging a hole in the tiles demo.
 - W, A, S, D, Ctrl, Spacebar: Control the camera position in 3D mode.
 - Arrow keys: Control the character in 2D mode.
 - W, S: Control the zoom in 2D mode.
 - Escape: Terminate the client worker.
 - F7: Toggle between static interest and dynamic interest updates.
 - F8: Toggle between 2D mode and 3D mode independent of running demo.
 - F9: Dump a worker view dependency graph to a file in [GraphViz DOT format](https://www.graphviz.org/doc/info/lang.html).
 - F10: Toggle between normal texture rendering and UV coordinate debug visualization mode.
 - F11: Toggle between windowed mode and fullscreen mode.
