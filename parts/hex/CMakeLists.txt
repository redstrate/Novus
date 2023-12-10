# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(hexpart STATIC)
target_sources(hexpart PRIVATE
        document/buffer/qhexbuffer.cpp
        document/buffer/qhexbuffer.h
        document/buffer/qmemorybuffer.cpp
        document/buffer/qmemorybuffer.h
        document/buffer/qmemoryrefbuffer.cpp
        document/buffer/qmemoryrefbuffer.h
        document/qhexcursor.cpp
        document/qhexcursor.h
        document/qhexdocument.cpp
        document/qhexdocument.h
        document/qhexmetadata.cpp
        document/qhexmetadata.h
        document/qhexrenderer.cpp
        document/qhexrenderer.h
        hexpart.cpp
        hexpart.h
        qhexview.cpp
        qhexview.h)
target_link_libraries(hexpart
        PUBLIC
        Physis::Physis
        Qt6::Core
        Qt6::Widgets)
target_include_directories(hexpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Novus::HexPart ALIAS hexpart)