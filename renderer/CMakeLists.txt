# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

find_package(spirv_cross_core REQUIRED)
find_package(spirv_cross_glsl REQUIRED)
find_package(SPIRV-Headers REQUIRED)
find_package(glslang) # for additional debug information in the GLSL

add_library(renderer STATIC)
target_sources(renderer
        PRIVATE
        include/baserenderer.h
        include/buffer.h
        include/camera.h
        include/device.h
        include/drawobject.h
        include/gamerenderer.h
        include/rendermanager.h
        include/scene.h
        include/shadermanager.h
        include/shaderstructs.h
        include/simplerenderer.h
        include/swapchain.h
        include/texture.h
        include/pass.h

        src/device.cpp
        src/gamerenderer.cpp
        src/imguipass.cpp
        src/imguipass.h
        src/rendermanager.cpp
        src/shadermanager.cpp
        src/simplerenderer.cpp
        src/swapchain.cpp)
qt_add_resources(renderer
        "shaders"
        PREFIX "/"
        FILES
        shaders/dummy.frag.spv
        shaders/imgui.frag.spv
        shaders/imgui.vert.spv
        shaders/mesh.frag.spv
        shaders/mesh.vert.spv
        shaders/skinned.vert.spv
        shaders/blit.vert.spv
        shaders/blit.frag.spv
        shaders/debug.vert.spv
        shaders/debug.frag.spv)
target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(renderer
        PUBLIC
        Qt6::Core
        Vulkan::Vulkan
        Physis::Physis
        glm::glm
        imgui
        dxbc
        spirv-cross-core
        spirv-cross-glsl)
target_compile_definitions(renderer PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_ENABLE_EXPERIMENTAL)
target_compile_options(renderer PUBLIC -fexceptions) # needed for spirv-cross and dxbc

if (glslang_FOUND)
    target_link_libraries(renderer
            PUBLIC
            glslang::SPIRV
            glslang::glslang-default-resource-limits)
    target_compile_definitions(renderer PRIVATE HAVE_GLSLANG)
endif ()

add_library(Novus::Renderer ALIAS renderer)