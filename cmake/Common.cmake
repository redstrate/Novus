# SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

macro(set_common_properties target)
    # Needed for stupid glslang-using libraries
    get_target_property(EXCLUDE_FROM_SANITIZERS ${target} EXCLUDE_FROM_SANITIZERS)

    if (ENABLE_SANITIZERS AND NOT EXCLUDE_FROM_SANITIZERS)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endmacro()
