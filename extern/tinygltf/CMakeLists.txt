find_package(nlohmann_json REQUIRED)
find_package(stb REQUIRED)

add_library(tinygltf STATIC)
target_sources(tinygltf PRIVATE src/tiny_gltf.cc)
target_include_directories(tinygltf PUBLIC include)
target_link_libraries(tinygltf PUBLIC nlohmann_json::nlohmann_json stb::stb)