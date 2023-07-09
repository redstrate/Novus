add_library(mdlpart STATIC mdlpart.cpp)
target_link_libraries(mdlpart PUBLIC physis z ${LIBRARIES} Qt5::Core Qt5::Widgets renderer NovusCommon)
target_include_directories(mdlpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})