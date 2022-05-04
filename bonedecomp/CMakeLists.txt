add_executable(bonedecomp
        src/main.cpp)
target_link_libraries(bonedecomp PUBLIC libxiv ${LIBRARIES} Qt5::Core Qt5::Widgets magic_enum)

install(TARGETS bonedecomp
        DESTINATION "${INSTALL_BIN_PATH}")

if(WIN32)
    get_target_property(QMAKE_EXE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)

    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    # Run windeployqt immediately after build
    add_custom_command(TARGET bonedecomp
            POST_BUILD
            COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:bonedecomp>\"
            )
endif()
