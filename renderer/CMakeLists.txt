# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

find_package(spirv_cross_core REQUIRED)
find_package(spirv_cross_glsl REQUIRED)
find_package(SPIRV-Headers REQUIRED)
find_package(glslang) # For additional debug information in the GLSL (currently optional for the Windows CI build)

add_library(renderer STATIC)
set_target_properties(renderer
        PROPERTIES
        EXCLUDE_FROM_SANITIZERS ON) # Excluded from sanitizers because of glslang...
set_common_properties(renderer)
target_sources(renderer
        PRIVATE
        include/baserenderer.h
        include/buffer.h
        include/camera.h
        include/device.h
        include/drawobject.h
        include/gamerenderer.h
        include/pass.h
        include/rendermanager.h
        include/scene.h
        include/shadermanager.h
        include/shaderstructs.h
        include/simplerenderer.h
        include/swapchain.h
        include/texture.h
        include/frustum.h
        include/vfxpass.h
        include/vfxobject.h

        src/device.cpp
        src/gamerenderer.cpp
        src/imguipass.cpp
        src/imguipass.h
        src/rendermanager.cpp
        src/scene.cpp
        src/shadermanager.cpp
        src/simplerenderer.cpp
        src/swapchain.cpp
        src/frustum.cpp
        src/vfxpass.cpp)
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
        shaders/debug.frag.spv
        shaders/billboard.vert.spv
        shaders/billboard.frag.spv
        shaders/vfx.vert.spv
        shaders/vfx.frag.spv)
target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(renderer
        PUBLIC
        Qt6::Core
        Qt6::Gui
        Vulkan::Vulkan
        Physis::Physis
        glm::glm
        imgui
        dxbc
        spirv-cross-core
        spirv-cross-glsl
        Novus::Common)
target_compile_definitions(renderer PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_ENABLE_EXPERIMENTAL)
target_compile_options(renderer PUBLIC -fexceptions) # Exceptions are needed for SPIRV-Cross and DXBC

if (glslang_FOUND)
    target_link_libraries(renderer
            PUBLIC
            glslang::SPIRV
            glslang::glslang-default-resource-limits)
    target_compile_definitions(renderer PRIVATE HAVE_GLSLANG)
endif ()

add_library(Novus::Renderer ALIAS renderer)
