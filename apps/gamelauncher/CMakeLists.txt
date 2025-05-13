# SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-gamelauncher)
target_sources(novus-gamelauncher
        PRIVATE
        include/mainwindow.h

        src/main.cpp
        src/mainwindow.cpp)
target_include_directories(novus-gamelauncher
        PUBLIC
        include)
target_link_libraries(novus-gamelauncher
        PRIVATE
        Novus::Common
        Physis::Physis
        KF6::I18n
        Qt6::Core
        Qt6::Widgets)

install(TARGETS novus-gamelauncher ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES gamelauncher.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/novus)

if (WIN32)
    set_target_properties(novus-gamelauncher PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "GameLauncher")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-gamelauncher> DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
