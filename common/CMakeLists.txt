add_library(NovusCommon STATIC)
target_sources(NovusCommon PRIVATE
        include/aboutwindow.h
        include/filecache.h
        src/aboutwindow.cpp
        src/filecache.cpp)
target_include_directories(NovusCommon PUBLIC
        include
        PRIVATE
        ${CMAKE_BINARY_DIR})
target_link_libraries(NovusCommon PUBLIC Qt5::Core Qt5::Widgets physis)

# meant for including the license text
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE LICENSE_TXT)
STRING(REPLACE "\n" " \\n" LICENSE_TXT ${LICENSE_TXT})
STRING(REPLACE "\"" "\"\"" LICENSE_TXT ${LICENSE_TXT})

configure_file(${CMAKE_CURRENT_LIST_DIR}/../cmake/license.h.in
        ${CMAKE_BINARY_DIR}/license.h)
