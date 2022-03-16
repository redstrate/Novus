project(Novus)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Core Widgets CONFIG REQUIRED)

add_subdirectory(libxiv)

find_package(fmt)

add_executable(novus
        src/main.cpp
        src/mainwindow.cpp)
target_include_directories(novus
        PUBLIC
        include)
target_link_libraries(novus PUBLIC libxiv fmt::fmt z  Qt5::Core Qt5::Widgets)