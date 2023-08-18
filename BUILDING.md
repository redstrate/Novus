# Building Novus

## Dependencies

### Required

All of these packages are required for Astra to build with a minimal set of features.

* Linux
    * Windows, macOS and other systems may work but are currently unsupported.
* CMake 3.25 or later
* Qt 5.15 or later
* Rust

## Configuring

**Note:** Some dependencies will automatically be downloaded from the Internet if not found on your system. This functionality may change in the future.

To configure, run `cmake` in the source directory:

```bash
$ cd novus
$ cmake -S . -B build
```

This command will create a new build directory and configure the source directory (`.`). If you want to enable more options, pass the mnow:

```bash
$ cmake -S . -B build -DSOME_OVERRIDE=ON
```

## Building

Now begin building the project:

```bash
$ cmake --build build
```

If the build was successful, you'll find each binary under their respective folder in `build/`.