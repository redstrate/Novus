# SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-patchdiff)
set_common_properties(novus-patchdiff)
target_sources(novus-patchdiff
        PRIVATE
        include/mainwindow.h
        include/difftreemodel.h
        include/difftreewidget.h

        src/main.cpp
        src/mainwindow.cpp
        src/difftreemodel.cpp
        src/difftreewidget.cpp)
target_include_directories(novus-patchdiff
        PUBLIC
        include)
target_link_libraries(novus-patchdiff
        PRIVATE
        Novus::Common
        Novus::HexPart
        Physis::Physis
        KF6::I18n
        Qt6::Core
        Qt6::Widgets
        KF6::WidgetsAddons)

install(TARGETS novus-patchdiff ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES patchdiff.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/novus)

if (WIN32)
    set_target_properties(novus-patchdiff PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "PatchDiff")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-patchdiff> DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
