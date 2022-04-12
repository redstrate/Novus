find_package(Vulkan REQUIRED)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
)

FetchContent_MakeAvailable(glm)

add_library(renderer src/renderer.cpp)
target_include_directories(renderer PUBLIC include)
target_link_libraries(renderer PUBLIC Vulkan::Vulkan fmt::fmt libxiv glm::glm)