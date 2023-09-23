add_library(tinygltf STATIC)
target_sources(tinygltf PRIVATE src/tiny_gltf.cc)
target_include_directories(tinygltf PUBLIC include)