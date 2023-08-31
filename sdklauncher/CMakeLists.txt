set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(sdklauncher
        src/main.cpp
        src/mainwindow.cpp)
target_link_libraries(sdklauncher PUBLIC Qt5::Widgets)
target_include_directories(sdklauncher PUBLIC include)
