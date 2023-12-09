# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(novus-sagasu-static STATIC)
target_sources(novus-sagasu-static PRIVATE
        include/hashdatabase.h
        src/hashdatabase.cpp)
target_link_libraries(novus-sagasu-static PUBLIC physis z Qt6::Core Qt6::Sql novus-common)
target_include_directories(novus-sagasu-static PRIVATE include)

add_executable(novus-sagasu-indexer)
target_sources(novus-sagasu-indexer PRIVATE
        src/indexer.cpp)
target_link_libraries(novus-sagasu-indexer PRIVATE Qt6::HttpServer novus-sagasu-static)
target_include_directories(novus-sagasu-indexer PRIVATE include)

add_executable(novus-sagasu)
target_sources(novus-sagasu PRIVATE
        src/main.cpp
        src/mainwindow.cpp
        src/filetreewindow.cpp
        src/filepropertieswindow.cpp
        src/filetreemodel.cpp)
target_include_directories(novus-sagasu PRIVATE include)
target_link_libraries(novus-sagasu PRIVATE Qt6::Concurrent physis-logger sklbpart cmppart shpkpart hexpart exlpart mdlpart exdpart texpart novus-sagasu-static)

install(TARGETS novus-sagasu ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
