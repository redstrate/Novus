# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-mdlviewer
        src/main.cpp
        src/mainwindow.cpp)
target_include_directories(novus-mdlviewer
        PUBLIC
        include)
target_link_libraries(novus-mdlviewer PUBLIC physis z ${LIBRARIES} Qt6::Core Qt6::Widgets KF6::XmlGui mdlpart novus-common)

install(TARGETS novus-mdlviewer ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

if(WIN32)
    get_target_property(QMAKE_EXE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET novus-mdlviewer
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:novus-mdlviewer>\"
            )
endif()
