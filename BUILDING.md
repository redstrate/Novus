# Building Novus

Currently the only way to build Novus is manually.

## Manual

### Dependencies

* [Linux](https://kernel.org/)
  * Windows, macOS and other systems may work but are currently unsupported. Patches are accepted to fix any problems with those OSes though.
* [CMake](https://cmake.org) 3.25 or later
* [Qt](https://www.qt.io/) 6.6 or later
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

* [Qt 6.6](https://doc.qt.io/qt-6/get-and-install-qt.html) (with support for the HTTP Server and WebSockets)
* [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows)
* [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/) (for a C++ compiler)
* CMake (can be installed with the Visual Studio Build Tools)

Make sure your Qt bin (like `C:\Qt\6.7.0\msvc2019_64\bin`) is in your `PATH` environment variable before building, otherwise Qt will not be picked up by CMake.

Afterwards, run `.\scripts\windows-setup.ps1` and `.\scripts\windows-build.ps1` in PowerShell.
