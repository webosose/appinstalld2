# Copyright (c) 2017-2018 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

#
# appinstalld/CMakeLists.txt
#

cmake_minimum_required(VERSION 2.8.7)

project(appinstalld C CXX)

set(CMAKE_BUILD_TYPE Debug)

include(webOS/webOS)
webos_modules_init(1 5 0)
webos_component(1 0 0)

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS_OTHER})

pkg_check_modules(LUNASERVICE2PP REQUIRED luna-service2++)
include_directories(${LUNASERVICE2PP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2PP_CFLAGS_OTHER})

pkg_check_modules(PBNJSON_CPP REQUIRED pbnjson_cpp)
include_directories(${PBNJSON_CPP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PBNJSON_CPP_CFLAGS_OTHER})

pkg_check_modules(PMLOG REQUIRED PmLogLib)
include_directories(${PMLOG_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PMLOG_CFLAGS_OTHER})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${Boost_CFLAGS_OTHER})
add_definitions(-DBOOST_BIND_NO_PLACEHOLDERS)

pkg_check_modules(PMTRACE REQUIRED PmTrace)
include_directories(${PMTRACE_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PMTRACE_CFLAGS_OTHER})
add_definitions(-DBOOST_BIND_NO_PLACEHOLDERS)

find_library(ICU NAMES icuuc)
if(ICU STREQUAL "ICU-NOTFOUND")
   message(FATAL_ERROR "Failed to find ICU4C libraries. Please install.")
endif()

webos_add_compiler_flags(DEBUG -O0)
webos_add_compiler_flags(ALL -Wall "-std=c++11")
add_definitions(-D__STDC_FORMAT_MACROS)

# Require that all undefined symbols are satisfied by the libraries from target_link_libraries()
webos_add_linker_options(ALL --no-undefined)

webos_include_install_paths()

# Build the appinstalld executable
file(GLOB_RECURSE SOURCES src/*.cpp)
include_directories(src)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

include_directories(${WEBOS_BINARY_CONFIGURED_DIR})

SET (EXT_LIBS
    ${GLIB2_LDFLAGS}
    ${LUNASERVICE2PP_LDFLAGS}
    ${PBNJSON_CPP_LDFLAGS}
    ${PMLOG_LDFLAGS}
    ${Boost_LIBRARIES}
    ${ICU}
    ${PMTRACE_LDFLAGS}
)

target_link_libraries(${CMAKE_PROJECT_NAME} ${EXT_LIBS})

set(webos_program_NAME ${CMAKE_PROJECT_NAME})
set(permissions PERMISSIONS OWNER_READ OWNER_EXECUTE)
_webos_set_bin_inst_dir(destdir TRUE FALSE)
install(TARGETS ${webos_program_NAME} DESTINATION ${destdir} ${permissions})

webos_build_system_bus_files()
webos_build_configured_file(files/launch/appinstalld.conf SYSCONFDIR init)

file(GLOB_RECURSE SCHEMAS files/schema/*.schema)
install(FILES ${SCHEMAS} DESTINATION ${WEBOS_INSTALL_WEBOS_SYSCONFDIR}/schemas/appinstalld)
if(EXISTS ${CMAKE_SOURCE_DIR}/files/conf/appinstalld-conf.json.in)
  webos_build_configured_file(${CMAKE_SOURCE_DIR}/files/conf/appinstalld-conf.json WEBOS_SYSCONFDIR "")
endif()

file(GLOB_RECURSE OPKG files/opkg/opkg.conf)
install(FILES ${OPKG} DESTINATION ${WEBOS_INSTALL_WEBOS_SYSCONFDIR}/appinstalld)

webos_config_build_doxygen(files/doc Doxyfile)
