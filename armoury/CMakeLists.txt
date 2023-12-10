# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-armoury)
target_sources(novus-armoury
        PRIVATE
        include/boneeditor.h
        include/cmpeditor.h
        include/fullmodelviewer.h
        include/gearlistmodel.h
        include/gearlistwidget.h
        include/gearview.h
        include/mainwindow.h
        include/penumbraapi.h
        include/settingswindow.h
        include/singlegearview.h

        src/boneeditor.cpp
        src/cmpeditor.cpp
        src/fullmodelviewer.cpp
        src/gearlistmodel.cpp
        src/gearlistwidget.cpp
        src/gearview.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/penumbraapi.cpp
        src/settingswindow.cpp
        src/singlegearview.cpp)
target_include_directories(novus-armoury
        PUBLIC
        include
        PRIVATE
        ${CMAKE_BINARY_DIR})
target_link_libraries(novus-armoury
        PUBLIC
        Novus::Common
        Novus::MdlPart
        Novus::CmpPart
        Novus::SklbPart
        Physis::Physis
        Physis::Logger
        imgui
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
        Qt6::Network)

install(TARGETS novus-armoury ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
