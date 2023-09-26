# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.25)
project(Novus VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION 5.15)
set(KF_MIN_VERSION 5.100)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(ECMFindQmlModule)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPoQmTools)
include(KDEGitCommitHooks)
include(KDEClangFormat)
include(FeatureSummary)
include(FetchContent)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX NOVUS
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/novus-version.h
)

find_package(Qt6 ${QT_MIN_VERSION} COMPONENTS Core Widgets Concurrent Core5Compat CONFIG REQUIRED)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS CoreAddons Config XmlGui)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
if (NOT TARGET glm::glm)
    add_library(glm::glm ALIAS glm)
endif()

add_subdirectory(extern)

# macos
if(APPLE)
    set(USE_STANDALONE_WINDOW TRUE)
endif()

add_subdirectory(renderer)
add_subdirectory(exdviewer)
add_subdirectory(armoury)
add_subdirectory(argcracker)
add_subdirectory(explorer)
#add_subdirectory(bonedecomp) # not yet ported to physis
add_subdirectory(parts)
add_subdirectory(common)
add_subdirectory(mdlviewer)
add_subdirectory(sdklauncher)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)