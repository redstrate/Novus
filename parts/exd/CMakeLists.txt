add_library(exdpart STATIC exdpart.cpp)
target_link_libraries(exdpart PUBLIC physis z ${LIBRARIES} Qt5::Core Qt5::Widgets)
target_include_directories(exdpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})