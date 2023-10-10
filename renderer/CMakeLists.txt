# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(renderer STATIC)
target_sources(renderer PRIVATE
        src/renderer.cpp
        src/imguipass.cpp
        src/imguipass.h)
qt_add_resources(renderer "shaders"
        PREFIX "/"
        FILES
        shaders/imgui.frag.spv
        shaders/imgui.vert.spv
        shaders/mesh.frag.spv
        shaders/mesh.vert.spv)
target_include_directories(renderer PUBLIC include)
target_link_libraries(renderer PUBLIC Qt6::Core Vulkan::Vulkan physis z glm::glm imgui)
target_compile_definitions(renderer PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE)
