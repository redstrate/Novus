FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG v0.2.2
)

FetchContent_MakeAvailable(Corrosion)

FetchContent_Declare(
        libphysis
        GIT_REPOSITORY https://git.sr.ht/~redstrate/libphysis
        GIT_TAG        0.1.1
)

FetchContent_MakeAvailable(libphysis)

corrosion_import_crate(MANIFEST_PATH ${libphysis_SOURCE_DIR}/Cargo.toml)

target_include_directories(physis INTERFACE ${libphysis_SOURCE_DIR}/target/public)
target_link_libraries(physis INTERFACE unshield)

add_subdirectory(magic_enum)