# PlayerModelManager Filesystem Models

An addon for [PlayerModelManager](https://github.com/YAZ64MT/PlayerModelManager) that allows Link models to be loaded from the file system.

It has the following dependencies:
* [YAZMT Core](https://github.com/YAZ64MT/CoreLib)
* [YAZMT GlobalObjects](https://github.com/YAZ64MT/GlobalObjects)
* [YAZMT PlayerModelManager](https://github.com/YAZ64MT/PlayerModelManager)

## Usage
This mod can currently only load models that were compiled with OoTO/MMO zzplayas/z64playas manifests. The largest repository of such models can be found [here](https://github.com/hylian-modding/Z64-CustomPlayerModels/tree/master/manifests). Do NOT report model issues to the HylianModding repository UNLESS you can verify the issue also appears in ModLoader64—compatibility is not perfect between PMM and the models from that repository.

Models are commonly found with the file extension `.zobj`, but not all `.zobj` files are valid models. Most models with the ASCII `MODLOADER64` at offset 0x5000 are compatible though.

For first time setup, first drag the zip containing this mod and any dependencies into the mod menu of Zelda64: Recompiled and enable them. Then, run the game, let it get to the title screen. Hit the "Open Mods Folder" button in the mods menu. If you go up one level in the file system, you should see the folders `mods`, `saves`, and the newly-created `mod_data` folder. Within the `mod_data` folder, you will find a folder named `yazmt_z64_playermodelmanager`. You must place models in the `models` folder within `yazmt_z64_playermodelmanager`. Models will only be loaded when the game boots up, so if you add new models to the folder, make sure to restart the game.

You may notice incorrect textures or missing equipment on certain models, particularly models made for Ocarina of Time. This is because those models rely on assets from OoT. If you wish to correct these errors, place a supported OoT ROM named `oot.z64`, `oot.n64`, or `oot.v64`. into the `yazmt_z64_playermodelmanager` folder. If you want to check if your ROM is supported before placing it in the folder, make sure it is in big endian (.z64) format by using a tool such as [this online swapper provided by Hack64](https://hack64.net/tools/swapper.php) or [Tool64](https://gbatemp.net/download/tool-64.32494/). Just changing the file extension does NOT make it big endian. After converting your ROM to big endian if needed, you can manually check if its SHA1 hash matches with the list of supported versions below.

The following OoT ROMs are supported:
```
Version | Region |                  SHA-1
  1.0   |   US   | ad69c91157f6705e8ab06c79fe08aad47bb57ba7
  1.1   |   US   | d3ecb253776cd847a5aa63d859d8c89a2f37b364
  1.2   |   US   | 41b3bdc48d98c48529219919015a1af22f5057c2
  1.0   |   JP   | c892bbda3993e66bd0d56a10ecd30b1ee612210f
  1.1   |   JP   | dbfc81f655187dc6fefd93fa6798face770d579d
  1.2   |   JP   | fa5f5942b27480d60243c2d52c0e93e26b9e6b86
```


## Writing mods

See [this document](https://hackmd.io/fMDiGEJ9TBSjomuZZOgzNg) for an explanation of the modding framework, including how to write function patches and perform interop between different mods.

## Tools

This template has somewhat different requirements from the default mod template. In order to run it, you'll need the following:

* `make`
* `cmake`
* `ninja`
* `python` (or `python3` on POSIX systems).
* `zig`

On Linux and MacOS, you'll need to also ensure that you have the `zip` utility installed.

All of these can (and should) be installed via using [chocolatey](https://chocolatey.org/) on Windows, Homebrew on MacOS, or your distro's package manager on Linux.

**You do NOT need the `RecompModTool` tool, as the build script will compile all of the N64Recomp tools for you.**

You'll also need a `gcc` compatible compiler and linker with MIPS support. `clang` and `ld.lld` (part of the llvm toolset) are recommended.

* On Windows, using [chocolatey](https://chocolatey.org/) to install both is recommended. The packages are `llvm` and `make` respectively.
  * The LLVM 19.1.0 [llvm-project](https://github.com/llvm/llvm-project) release binary, which is also what chocolatey provides, does not support MIPS correctly. The solution is to install 18.1.8 instead, which can be done in chocolatey by specifying `--version 18.1.8` or by downloading the 18.1.8 release directly.
* On Linux, these can both be installed using your distro's package manager.
* On MacOS, these can both be installed using Homebrew. Apple clang won't work, as you need a mips target for building the mod code.

Alternatively, if you don't want to downgrade your clang version (or want a later version than what's provided for your system), I maintain [MIPS-only builds of the latest llvm utilities](https://github.com/LT-Schmiddy/n64recomp-clang/releases/latest). They're what I use. I recommend the `N64RecompEssentials` packages, as they only have the tools that are useful for working with recomp mods.

## Building

Run `git submodule update --init --recursive` to make sure you've clones all submodules. Then, run `make` (with an optional job count) to build everything.

Alternatively, you can also use special target invokations to only build specific things, such as the mod `.nrm` file or the extlib for a specific platform.
See the Makefile for all valid targets.

This repo is set up for building an extlib by default. If your mod isn't meant to have an extlib, simply remove the `extlib_compilation` section from your
`mod.toml`, and the build scripts will disable all extlib handling when building the `all` target.

On your first run, a file called `user_build_config.json` will be created at the root of the repo. Here you can set the command/path for the compiler and linker
you want to use for your mod code. If you want to use my MIPS-only clang builds (or any compiler not on the system path), this is an easy way to set them up.
You can also set which CMake presets you want to use for building your external library.

## Extlib Compilation, Cross-Compilation, and CMake Presets

This template is set up to automatically build and cross-compile your extlib code (via CMake and Zig) alongside your mod code when you invoke `make`.
CMake presets are used to handle any configuration differences between target platforms ([More info about CMake presets can be found
here](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)). The default presets to use for each can be set in the `extlib_compiling`
section of your `mod.toml`. These are copied into `user_build_config.json` when the file is created. If you want to use a different preset for
a platform when compiling on your local system, without changing the default, you can change the preset in `user_build_config.json`.

More information about extlib compiling can be found in the `mod.toml` file.

## Testing

This template includes handling of a dedicated testing environment for you mod in the form of the `./runtime` folder. First, create `./runtime` in your
mod's root directory and copy in recomp's `assets` directory into it (You can also copy anyu config files, saves, and other mods you want to test against). After a build, your mod's `.nrm` file (and extlib file, if one is being built) will be copied to a folder called `./runtime/mods`, and a file called `./runtime/portable.txt` will be created. Then, if you use `runtime` and as your CWD, everything's ready to go for immediate testing as soon as your build finishes.

## Updating the Majora's Mask Decompilation Submodule

Mods can also be made with newer versions of the Majora's Mask decompilation instead of the commit targeted by this repo's submodule.
To update the commit of the decompilation that you're targeting, follow these steps:

* Build the [N64Recomp](https://github.com/N64Recomp/N64Recomp) repo and copy the N64Recomp executable to the root of this repository.
* Build the version of the Majora's Mask decompilation that you want to update to and copy the resulting .elf file to the root of this repository.
* Update the `mm-decomp` submodule in your clone of this repo to point to the commit you built in the previous step.
* Run `N64Recomp generate_symbols.toml --dump-context`
* Rename `dump.toml` and `data_dump.toml` to `mm.us.rev1.syms.toml` and `mm.us.rev1.datasyms.toml` respectively.
  * Place both files in the `Zelda64RecompSyms` folder.
* Try building.
  * If it succeeds, you're done.
  * If it fails due to a missing header, create an empty header file in the `include/dummy_headers` folder, with the same path.
    * For example, if it complains that `assets/objects/object_cow/object_cow.h` is missing, create an empty `include/dummy_headers/objects/object_cow.h` file.
  * If RecompModTool fails due to a function "being marked as a patch but not existing in the original ROM", it's likely that function you're patching was renamed in the Majora's Mask decompilation.
    * Find the relevant function in the map file for the old decomp commit, then go to that address in the new map file, and update the reference to this function in your code with the new name.
