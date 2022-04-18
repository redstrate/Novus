find_package(assimp REQUIRED)

add_executable(mdlviewer
        src/main.cpp
        src/mainwindow.cpp)
target_include_directories(mdlviewer
        PUBLIC
        include)
target_link_libraries(mdlviewer PUBLIC
        libxiv
        ${LIBRARIES}
        Qt5::Core
        Qt5::Widgets
        renderer
        assimp::assimp
        magic_enum)

install(TARGETS mdlviewer
        DESTINATION "${INSTALL_BIN_PATH}")

if(WIN32)
    get_target_property(QMAKE_EXE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET mdlviewer
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:mdlviewer>\"
            )
endif()
