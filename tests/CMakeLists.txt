find_package(GTest REQUIRED)
enable_testing()

add_executable(${PROJECT_NAME}-test)
get_target_property(LIB_SOURCES ${PROJECT_NAME} SOURCES)

target_sources(${PROJECT_NAME}-test
  PRIVATE
    ${LIB_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/test-main.c++
)

target_link_libraries(${PROJECT_NAME}-test
  PRIVATE
    GTest::GTest
    GTest::Main
    ${PROJECT_NAME}
)
