find_package(Vulkan REQUIRED)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
)

FetchContent_MakeAvailable(glm)

if(USE_STANDALONE_WINDOW)
    find_package(SDL2 REQUIRED)
    set(EXTRA_SRC src/standalonewindow.cpp)
    set(EXTRA_LIBRARIES SDL2::SDL2)
endif()

add_library(renderer src/renderer.cpp ${EXTRA_SRC})
target_include_directories(renderer PUBLIC include)
target_link_libraries(renderer PUBLIC Vulkan::Vulkan fmt::fmt libxiv glm::glm ${EXTRA_LIBRARIES})
target_compile_definitions(renderer PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE)

if(USE_STANDALONE_WINDOW)
    target_compile_definitions(renderer PUBLIC USE_STANDALONE_WINDOW)
endif()