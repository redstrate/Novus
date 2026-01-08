# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

# config file used for app executable names
# TODO: haha this is horrible, make this easier pls
file (GENERATE
        OUTPUT "launcherconfig_$<CONFIG>.h.out"
        CONTENT "\
#pragma once\n\
#define GEAREDITOR_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-armoury>\")\n\
#define EXCELEDITOR_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-karuku>\")\n\
#define MAPEDITOR_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-mapeditor>\")\n\
#define MATEDITOR_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-mateditor>\")\n\
#define MDLVIEWER_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-mdlviewer>\")\n\
#define DATAEXPLORER_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-sagasu>\")\n\
        "
)

# copy the new file to something standard we'll plug into the C++
add_custom_command(
        COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" "launcherconfig_$<CONFIG>.h.out" "launcherconfig.h"
        VERBATIM
        PRE_BUILD
        DEPENDS  "launcherconfig_$<CONFIG>.h.out"
        OUTPUT   "launcherconfig.h"
        COMMENT  "creating config.out file ({event: PRE_BUILD}, {filename: config.out})"
)

if (WIN32)
    add_library(novus-common SHARED)
else()
    add_library(novus-common STATIC)
endif()
set_common_properties(novus-common)
target_sources(novus-common
        PRIVATE
        include/aboutdata.h
        include/filecache.h
        include/filetypes.h
        include/quaternionedit.h
        include/settings.h
        include/vec3edit.h
        include/pathedit.h
        include/enumedit.h

        src/aboutdata.cpp
        src/filecache.cpp
        src/filetypes.cpp
        src/quaternionedit.cpp
        src/settings.cpp
        src/vec3edit.cpp
        src/pathedit.cpp
        src/enumedit.cpp

        launcherconfig.h)
target_include_directories(novus-common
        PUBLIC
        include
        PRIVATE
        ${CMAKE_BINARY_DIR})
target_link_libraries(novus-common
        PUBLIC
        Physis::Physis
        KF6::CoreAddons
        KF6::ConfigCore
        KF6::XmlGui
        KF6::I18n
        Qt6::Core
        Qt6::Widgets
        glm::glm
        magic_enum)
target_compile_definitions(novus-common PRIVATE TRANSLATION_DOMAIN="novus")
set_target_properties(novus-common PROPERTIES
        EXPORT_NAME NovusCommon
)
ecm_generate_export_header(novus-common
        BASE_NAME NovusCommon
        VERSION 1.0.0
)

if (WIN32)
    target_link_libraries(novus-common PRIVATE
            KF6::BreezeIcons
    )
endif ()

target_include_directories(novus-common PUBLIC "$<BUILD_INTERFACE:${novus-common_INCLUDE_DIRS}>")

add_library(Novus::Common ALIAS novus-common)

if (WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:novus-common> DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
