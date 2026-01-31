# Building Novus

There are two methods to build Novus, either via [Flatpak](https://flatpak.org/) or manually using your system libraries. It's highly recommended to prefer the Flatpak, especially if you don't have experience with CMake, C++ and such.

## Flatpak

Building the Flatpak version is easy, and there's a helper script to speed up the process. You must run it from the repository root:

```shell
cd novus
./scripts/build-flatpak.sh
```

The process should only take a few minutes on a moderately powerful machine. It does require an Internet connection and the relevant permissions to install the required Flatpak runtimes and extensions.

When it's complete, a file called `novus.flatpak` will appear in the repository root and that can be installed with the `flatpak` CLI tool or your preferred application store.

## Manual

### Dependencies

* [Linux](https://kernel.org/) or Windows
  * macOS may work but is currently unsupported. Patches are accepted to fix any problems with macOS builds though.
* [CMake](https://cmake.org) 3.25 or later
* [Qt](https://www.qt.io/) 6.9 or later
* [KDE Frameworks](https://develop.kde.org/products/frameworks/) 6
* [Rust](https://www.rust-lang.org/)
* [Corrosion](https://github.com/corrosion-rs/corrosion)

### Getting source code

Novus has git submodules that must be cloned alongside the repository, so make sure to pass the `--recursive` flag:

```bash
$ git clone --recursive https://github.com/redstrate/Novus.git
```

If you missed it, it's possible to initialize the submodules afterward:

```bash
$ git submodule update --init --recursive
```

### Configuring

To configure, run `cmake` in the source directory:

```bash
$ cd Novus
$ cmake -S . -B build
```

This command will create a new build directory and configure the source directory (`.`). If you want to enable more options, pass them now:

```bash
$ cmake -S . -B build -DENABLE_SOMETHING=ON
```

## Building

Now begin building the project:

```bash
$ cmake --build build
```

If the build was successful, the tool binaries will be found in `./build/bin`.

## Windows

Install the following:

* [Qt 6.6](https://doc.qt.io/qt-6/get-and-install-qt.html)
* [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows)
* [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/) (for a C++ compiler)
* CMake (can be installed with the Visual Studio Build Tools)
* [Rust](https://www.rust-lang.org/tools/install)

Make sure your Qt bin (like `C:\Qt\6.7.0\msvc2019_64\bin`) is in your `PATH` environment variable before building, otherwise Qt will not be picked up by CMake.

Afterwards, run `.\scripts\windows-setup.ps1` and `.\scripts\windows-build.ps1` in PowerShell.
