# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(mdlpart STATIC)
target_sources(mdlpart PRIVATE
        mdlexport.cpp
        mdlexport.h
        mdlimport.cpp
        mdlimport.h
        mdlpart.cpp
        mdlpart.h
        vulkanwindow.cpp
        vulkanwindow.h
)
target_link_libraries(mdlpart PUBLIC
        Novus::Renderer
        Novus::Common
        Physis::Physis
        Qt6::Core
        Qt6::Widgets
        tinygltf)
target_include_directories(mdlpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Novus::MdlPart ALIAS mdlpart)