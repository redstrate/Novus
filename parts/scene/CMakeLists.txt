# SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(scenepart STATIC)
set_common_properties(scenepart)
target_sources(scenepart
        PRIVATE
        animation.h
        collapsesection.h
        exceledit.h
        mapview.h
        objectidedit.h
        objectpass.h
        objectpropertieswidget.h
        primitives.h
        scenelistmodel.h
        scenelistwidget.h
        scenepart.h
        scenestate.h

        animation.cpp
        collapsesection.cpp
        exceledit.cpp
        mapview.cpp
        objectidedit.cpp
        objectpass.cpp
        objectpropertieswidget.cpp
        primitives.cpp
        scenelistmodel.cpp
        scenelistwidget.cpp
        scenepart.cpp
        scenestate.cpp)
target_link_libraries(scenepart
        PUBLIC
        Novus::Renderer
        Novus::Common
        Novus::MdlPart
        Novus::ExdPart
        Physis::Physis
        Qt6::Core
        Qt6::Widgets
        tinygltf)
target_include_directories(scenepart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(scenepart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::ScenePart ALIAS scenepart)
