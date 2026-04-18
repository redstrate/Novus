# SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-enemyeditor)
set_common_properties(novus-enemyeditor)
target_sources(novus-enemyeditor
        PRIVATE
        include/mainwindow.h
        include/enemymodel.h

        src/main.cpp
        src/mainwindow.cpp
        src/enemymodel.cpp)
target_include_directories(novus-enemyeditor
        PUBLIC
        include)
target_link_libraries(novus-enemyeditor
        PRIVATE
        Novus::MdlPart
        Novus::Common
        Physis::Physis
        KF6::XmlGui
        Qt6::Core
        Qt6::Widgets)

install(TARGETS novus-enemyeditor ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES enemyeditor.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/novus)

if (WIN32)
    set_target_properties(novus-enemyeditor PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "EnemyEditor")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-enemyeditor> DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES zone.xiv.enemyeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
endif()
