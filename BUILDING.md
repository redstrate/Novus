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

### Configuring

To configure, run `cmake` in the source directory:

```bash
$ cd novus
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