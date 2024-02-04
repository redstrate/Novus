# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.25)
project(Novus VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION 6.5)
set(KF_MIN_VERSION 5.240)

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

find_package(Qt6 ${QT_MIN_VERSION} COMPONENTS Core Widgets Concurrent Core5Compat Sql HttpServer Network CONFIG REQUIRED)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS CoreAddons Config XmlGui Archive I18n)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
if (NOT TARGET glm::glm)
    add_library(glm::glm ALIAS glm)
endif ()

add_subdirectory(extern)
add_subdirectory(renderer)
add_subdirectory(karuku)
add_subdirectory(armoury)
add_subdirectory(argcracker)
add_subdirectory(sagasu)
add_subdirectory(parts)
add_subdirectory(common)
add_subdirectory(mapeditor)
add_subdirectory(mdlviewer)
add_subdirectory(launcher)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)