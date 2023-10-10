# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(novus-sagasu
        src/main.cpp
        src/mainwindow.cpp
        src/filetreewindow.cpp
        src/filepropertieswindow.cpp)
target_include_directories(novus-sagasu
        PUBLIC
        include)
target_link_libraries(novus-sagasu PUBLIC physis z ${LIBRARIES} Qt6::Core Qt6::Widgets novus-common)

install(TARGETS novus-sagasu ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

if(WIN32)
    get_target_property(QMAKE_EXE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET novus-sagasu
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:novus-sagasu>\"
            )
endif()
