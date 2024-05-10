# SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-mateditor)
target_sources(novus-mateditor
        PRIVATE
        include/mainwindow.h
        include/materialview.h

        src/main.cpp
        src/mainwindow.cpp
        src/materialview.cpp)
target_include_directories(novus-mateditor
        PUBLIC
        include)
target_link_libraries(novus-mateditor
        PRIVATE
        Novus::Common
        Novus::MdlPart
        Novus::MtrlPart
        Physis::Physis
        Physis::Logger
        KF6::I18n
        Qt6::Core
        Qt6::Widgets)

install(TARGETS novus-mateditor ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

if (WIN32)
    set_target_properties(novus-mateditor PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "MaterialEditor")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-mateditor> DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()