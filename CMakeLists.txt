project(Novus)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Core Widgets CONFIG REQUIRED)

add_subdirectory(libxiv)

include(FetchContent)

find_package(fmt QUIET)

if(TARGET fmt::fmt)
    message("Using system library for fmt")

    set(LIBRARIES fmt::fmt ${LIBRARIES})
else()
    message("Using built-in fmt")

    FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt
            GIT_TAG        master
    )

    FetchContent_MakeAvailable(fmt)

    set(LIBRARIES fmt::fmt ${LIBRARIES})
endif()

# macos
if(UNIX AND NOT LINUX)
    set(USE_STANDALONE_WINDOW TRUE)
endif()

add_subdirectory(renderer)
add_subdirectory(exdviewer)
add_subdirectory(mdlviewer)