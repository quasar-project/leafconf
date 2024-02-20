cmake_minimum_required(VERSION 3.15)

include_guard(GLOBAL)

project(leafconf
  VERSION 0.1.0
  DESCRIPTION "Configuration file management library"
  HOMEPAGE_URL "io.whs31.leaf"
  LANGUAGES C CXX
)

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(cmake/all.cmake)

option(INTEGRATION_TESTS "Enable integration test" OFF)

set_language_standard(20)
check_language_features()

find_package(leaf 0.3.2 REQUIRED CONFIG)

add_library(${PROJECT_NAME} STATIC)
add_library(leaf::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# get_target_property(MY_PROJECT_SOURCES my_project SOURCES)
target_sources(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/leafconf/proxy.h> $<INSTALL_INTERFACE:include/leafconf/proxy.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/leafconf/serialization.h> $<INSTALL_INTERFACE:include/leafconf/serialization.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/leafconf/static.h> $<INSTALL_INTERFACE:include/leafconf/static.h>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/c++/serializers/toml.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/c++/dummy.c++
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    leaf::leaf
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    src/c++
)
target_add_metadata_definitions(${PROJECT_NAME})
enable_static_crt()

# ===  TESTS  ===
if(INTEGRATION_TESTS)
  add_subdirectory(tests)
endif()

# === INSTALL ===
set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/leafconfConfig.cmake.in")
set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/leafconfConfig.cmake")
set(config_targets_file "leafconfConfigTargets.cmake")
set(version_config_file "${CMAKE_CURRENT_BINARY_DIR}/leafconfConfigVersion.cmake")
set(export_dest_dir "${CMAKE_INSTALL_LIBDIR}/cmake/leafconf")

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}> $<INSTALL_INTERFACE:include/${PROJECT_NAME}> ;
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
export(TARGETS ${PROJECT_NAME}
  NAMESPACE leaf::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${config_targets_file}"
)
install(EXPORT ${PROJECT_NAME}
  DESTINATION "${export_dest_dir}"
  NAMESPACE leaf::
  FILE "${config_targets_file}"
)
configure_package_config_file(
  "${project_config_in}" "${project_config_out}"
  INSTALL_DESTINATION "${export_dest_dir}"
)
write_basic_package_version_file(
  "${version_config_file}"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)
install(FILES
  "${project_config_out}" "${version_config_file}"
  DESTINATION "${export_dest_dir}"
)
############################################################################

set(POST_BUILD_COMMENT "[${PROJECT_NAME}] build finished successfully (static library)")
add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ;
  COMMENT "${POST_BUILD_COMMENT}"
)

unset(INTEGRATION_TESTS CACHE)