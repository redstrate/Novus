# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-argcracker)
target_sources(novus-argcracker
        PRIVATE
        src/main.cpp
        src/tickcount.cpp)
target_link_libraries(novus-argcracker
        PRIVATE
        Physis::Physis
        Qt6::Core)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_compile_definitions(novus-argcracker PUBLIC MACOS)
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_definitions(novus-argcracker PUBLIC WIN32)
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_compile_definitions(novus-argcracker PUBLIC LINUX)
endif ()

install(TARGETS novus-argcracker ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

if (WIN32)
    set_target_properties(novus-argcracker PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "ArgCracker")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-argcracker> DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()