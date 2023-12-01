CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

OPTION(JSON_TEST "JSON_TEST" OFF)

project(json)

SET(JSON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src)

if(JSON_TEST)
    include(CTest)
    enable_testing()

    add_subdirectory(tests)
endif()