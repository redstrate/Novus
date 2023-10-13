# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-armoury)
target_sources(novus-armoury PRIVATE
        include/boneeditor.h
        include/cmpeditor.h
        include/fullmodelviewer.h
        include/gearlistmodel.h
        include/gearlistwidget.h
        include/gearview.h
        include/mainwindow.h
        include/quaternionedit.h
        include/singlegearview.h
        include/vec3edit.h

        src/boneeditor.cpp
        src/cmpeditor.cpp
        src/fullmodelviewer.cpp
        src/gearlistmodel.cpp
        src/gearlistwidget.cpp
        src/gearview.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/quaternionedit.cpp
        src/singlegearview.cpp
        src/vec3edit.cpp)

target_include_directories(novus-armoury
        PUBLIC
        include
        PRIVATE
        ${CMAKE_BINARY_DIR})
target_link_libraries(novus-armoury PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
        physis
        z
        physis-logger
        mdlpart
        cmppart
        imgui
        novus-common)

install(TARGETS novus-armoury ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
