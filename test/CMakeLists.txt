CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(json_test)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} json)