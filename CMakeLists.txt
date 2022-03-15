project(Novus)

add_subdirectory(libxiv)

find_package(fmt)

add_executable(novus
        src/main.cpp)
target_include_directories(novus
        PUBLIC
        include)
target_link_libraries(novus PUBLIC libxiv fmt::fmt z)