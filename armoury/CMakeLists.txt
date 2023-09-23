# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(armoury
        src/main.cpp
        src/mainwindow.cpp
        src/vec3edit.cpp
        include/vec3edit.h
        src/gearview.cpp
        src/singlegearview.cpp
        src/fullmodelviewer.cpp
        src/quaternionedit.cpp
        src/boneeditor.cpp
        src/cmpeditor.cpp
        src/gearlistwidget.cpp
        src/gearlistmodel.cpp)
target_include_directories(armoury
        PUBLIC
        include
        PRIVATE
        ${CMAKE_BINARY_DIR})
target_link_libraries(armoury PUBLIC
        ${LIBRARIES}
        Qt5::Core
        Qt5::Widgets
        Qt5::Concurrent
        magic_enum
        physis z
        mdlpart
        NovusCommon)

install(TARGETS armoury
        DESTINATION "${INSTALL_BIN_PATH}")

if(WIN32)
    get_target_property(QMAKE_EXE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET armoury
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:armoury>\"
            )
endif()