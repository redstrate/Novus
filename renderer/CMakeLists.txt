find_package(Vulkan REQUIRED)

add_library(renderer src/renderer.cpp)
target_include_directories(renderer PUBLIC include)
target_link_libraries(renderer PUBLIC Vulkan::Vulkan fmt::fmt)