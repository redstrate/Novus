# SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(scenepart STATIC)
target_sources(scenepart
        PRIVATE
        scenelistwidget.h
        scenelistmodel.h
        scenestate.h
        scenepart.h
        objectpropertieswidget.h
        collapsesection.h
        mapview.h
        objectpass.h
        primitives.h

        scenelistwidget.cpp
        scenelistmodel.cpp
        scenestate.cpp
        scenepart.cpp
        objectpropertieswidget.cpp
        collapsesection.cpp
        mapview.cpp
        objectpass.cpp
        primitives.cpp)
target_link_libraries(scenepart
        PUBLIC
        Novus::Renderer
        Novus::Common
        Novus::MdlPart
        Physis::Physis
        Qt6::Core
        Qt6::Widgets
        tinygltf)
target_include_directories(scenepart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(scenepart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::ScenePart ALIAS scenepart)
