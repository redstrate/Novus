# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

if(USE_STANDALONE_WINDOW)
    find_package(SDL2 REQUIRED)
    set(EXTRA_SRC src/standalonewindow.cpp)
    set(EXTRA_LIBRARIES SDL2::SDL2)
endif()

add_library(renderer src/renderer.cpp ${EXTRA_SRC})
target_include_directories(renderer PUBLIC include)
target_link_libraries(renderer PUBLIC Qt5::Core Vulkan::Vulkan physis z glm::glm ${EXTRA_LIBRARIES})
target_compile_definitions(renderer PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE)

if(USE_STANDALONE_WINDOW)
    target_compile_definitions(renderer PUBLIC USE_STANDALONE_WINDOW)
endif()