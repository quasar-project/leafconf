cmake_minimum_required(VERSION 3.15)

include_guard(GLOBAL)

project(leafconf
  VERSION 0.1.1
  DESCRIPTION "Configuration file management library"
  HOMEPAGE_URL "io.whs31.leaf"
  LANGUAGES C CXX
)

option(INTEGRATION_TESTS "Enable integration test" OFF)

include(dependencies/cmake/all.cmake)
enable_cross_compile_options()
set_language_standard(20)
check_language_features()

find_package(leaf 0.3.2 REQUIRED CONFIG)
find_package(tomlplusplus 3.4.0 REQUIRED CONFIG)

add_library(${PROJECT_NAME} STATIC)
add_library(leaf::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/leafconf/proxy.h> $<INSTALL_INTERFACE:include/leafconf/proxy.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/leafconf/serialization.h> $<INSTALL_INTERFACE:include/leafconf/serialization.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/leafconf/static.h> $<INSTALL_INTERFACE:include/leafconf/static.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/leafconf/serializers/toml.h> $<INSTALL_INTERFACE:include/leafconf/serializers/toml.h>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/c++/dummy.c++
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    leaf::leaf
    tomlplusplus::tomlplusplus
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
  target_add_integration_tests(${PROJECT_NAME}
    TESTS
      tests/test-main.c++
  )
endif()

# === INSTALL ===
target_install_library(${PROJECT_NAME} NAME leafconf NAMESPACE leaf)

unset(INTEGRATION_TESTS CACHE)
