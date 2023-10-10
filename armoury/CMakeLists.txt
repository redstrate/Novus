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

# note, this is temporary until we get dynamic skeleton generation. i really don't want to embed these
qt_add_resources(novus-armoury "skeletons"
        PREFIX "/"
        FILES
        skeletons/c0101_deform.json
        skeletons/c0101b0001.skel
        skeletons/c0201_deform.json
        skeletons/c0201b0001.skel
        skeletons/c0301_deform.json
        skeletons/c0301b0001.skel
        skeletons/c0401_deform.json
        skeletons/c0401b0001.skel
        skeletons/c0501_deform.json
        skeletons/c0501b0001.skel
        skeletons/c0601_deform.json
        skeletons/c0601b0001.skel
        skeletons/c0701_deform.json
        skeletons/c0701b0001.skel
        skeletons/c0701_deform.json
        skeletons/c0801_deform.json
        skeletons/c0801b0001.skel
        skeletons/c0901_deform.json
        skeletons/c0901b0001.skel
        skeletons/c1001_deform.json
        skeletons/c1001b0001.skel
        skeletons/c1101_deform.json
        skeletons/c1101b0001.skel
        skeletons/c1201_deform.json
        skeletons/c1201b0001.skel
        skeletons/c1301_deform.json
        skeletons/c1301b0001.skel
        skeletons/c1401_deform.json
        skeletons/c1401b0001.skel
        skeletons/c1501_deform.json
        skeletons/c1501b0001.skel
        skeletons/c1701_deform.json
        skeletons/c1701b0001.skel
        skeletons/c1801_deform.json
        skeletons/c1801b0001.skel)

target_include_directories(novus-armoury
        PUBLIC
        include
        PRIVATE
        ${CMAKE_BINARY_DIR})
target_link_libraries(novus-armoury PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
        magic_enum
        physis
        z
        physis-logger
        mdlpart
        imgui
        novus-common)

install(TARGETS novus-armoury ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

if(WIN32)
    get_target_property(QMAKE_EXE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET novus-armoury
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:novus-armoury>\"
            )
endif()