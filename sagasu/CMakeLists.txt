# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(novus-sagasu-static STATIC)
target_sources(novus-sagasu-static
        PRIVATE
        include/hashdatabase.h
        src/hashdatabase.cpp)
target_link_libraries(novus-sagasu-static
        PUBLIC
        Novus::Common
        Physis::Physis
        Qt6::Core
        Qt6::Sql)
target_include_directories(novus-sagasu-static PRIVATE include)

add_executable(novus-sagasu-indexer)
target_sources(novus-sagasu-indexer PRIVATE src/indexer.cpp)
target_link_libraries(novus-sagasu-indexer
        PRIVATE
        novus-sagasu-static
        Qt6::HttpServer)
target_include_directories(novus-sagasu-indexer PRIVATE include)

add_executable(novus-sagasu)
target_sources(novus-sagasu
        PRIVATE
        src/main.cpp
        src/mainwindow.cpp
        src/filetreewindow.cpp
        src/filepropertieswindow.cpp
        src/filetreemodel.cpp)
target_include_directories(novus-sagasu PRIVATE include)
target_link_libraries(novus-sagasu
        PRIVATE
        novus-sagasu-static
        Novus::SklbPart
        Novus::CmpPart
        Novus::ShpkPart
        Novus::HexPart
        Novus::ExlPart
        Novus::MdlPart
        Novus::ExdPart
        Novus::TexPart
        Physis::Logger
        Qt6::Concurrent)

install(FILES zone.xiv.sagasu.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES zone.xiv.sagasu.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(TARGETS novus-sagasu ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
