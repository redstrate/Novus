# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG v0.3.5
)

FetchContent_MakeAvailable(Corrosion)

FetchContent_Declare(
        libphysis
        GIT_REPOSITORY https://git.sr.ht/~redstrate/libphysis
        GIT_TAG        main
)

FetchContent_MakeAvailable(libphysis)

corrosion_import_crate(MANIFEST_PATH ${libphysis_SOURCE_DIR}/Cargo.toml)

target_include_directories(physis INTERFACE ${libphysis_SOURCE_DIR}/target/public)
target_link_libraries(physis INTERFACE unshield)

add_subdirectory(magic_enum)