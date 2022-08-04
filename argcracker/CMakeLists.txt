add_executable(argcracker src/main.cpp src/tickcount.cpp)
target_link_libraries(argcracker PUBLIC Qt5::Core ${LIBRARIES} physis z)
target_include_directories(argcracker PUBLIC include)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_compile_definitions(argcracker PUBLIC MACOS)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_definitions(argcracker PUBLIC WIN32)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_compile_definitions(argcracker PUBLIC LINUX)
endif()