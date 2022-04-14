add_executable(argcracker src/main.cpp src/tickcount.cpp)
target_link_libraries(argcracker PUBLIC Qt5::Core ${LIBRARIES})
target_include_directories(argcracker PUBLIC include)

if(APPLE)
    target_compile_definitions(argcracker PUBLIC MACOS)
endif()

if(WIN32)
    target_compile_definitions(argcracker PUBLIC WIN32)
endif()

if(LINUX)
    target_compile_definitions(argcracker PUBLIC LINUX)
endif()