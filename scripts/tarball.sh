#!/bin/sh

# ensure the submodules are up to date
git submodule init
git submodule update

# begin vendoring cargo dependencies
cd extern/libphysis
cargo vendor ../../cargo-vendored
mkdir .cargo
cp ../../scripts/config.toml .cargo/config.toml
cd ../../

tar --exclude='cmake-build*' --exclude='.idea' --exclude='.clang-format' --exclude='novus-source.tar.gz' --exclude-vcs -zcvf ../novus-source.tar.gz .
