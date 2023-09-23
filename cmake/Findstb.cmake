find_path(
        STB_INCLUDE_DIR
        NAMES stb_image.h
        PATH_SUFFIXES stb
)
find_package_handle_standard_args(stb DEFAULT_MSG STB_INCLUDE_DIR)
if(STB_FOUND)
    mark_as_advanced(STB_INCLUDE_DIR)
    add_library(stb INTERFACE)
    target_include_directories(stb SYSTEM INTERFACE ${STB_INCLUDE_DIR})
    add_library(stb::stb ALIAS stb)
endif()