find_package(assimp REQUIRED)
set_target_properties(assimp::assimp PROPERTIES MAP_IMPORTED_CONFIG_DEBUG Release)

add_library(mdlpart STATIC mdlpart.cpp)
target_link_libraries(mdlpart PUBLIC physis z ${LIBRARIES} Qt5::Core Qt5::Widgets renderer NovusCommon assimp::assimp)
target_include_directories(mdlpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})