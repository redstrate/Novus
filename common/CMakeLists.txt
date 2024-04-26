# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

if (WIN32)
    add_library(novus-common SHARED)
else()
    add_library(novus-common STATIC)
endif()
target_sources(novus-common
        PRIVATE
        include/aboutdata.h
        include/filecache.h
        include/filetypes.h
        include/novusmainwindow.h
        include/quaternionedit.h
        include/settings.h
        include/vec3edit.h

        src/aboutdata.cpp
        src/filecache.cpp
        src/filetypes.cpp
        src/novusmainwindow.cpp
        src/quaternionedit.cpp
        src/settings.cpp
        src/vec3edit.cpp)
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
        glm::glm)
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