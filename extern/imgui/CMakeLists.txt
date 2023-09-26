add_library(imgui STATIC)
target_sources(imgui PRIVATE
        include/imconfig.h
        include/imgui.h

        src/imgui.cpp
        src/imgui_demo.cpp
        src/imgui_draw.cpp
        src/imgui_internal.h
        src/imgui_tables.cpp
        src/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(imgui PUBLIC stb::stb)