CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

OPTION(JSON_BINARY "JSON_BINARY" OFF)
OPTION(JSON_TEST "JSON_TEST" OFF)
OPTION(JSON_INSTALL "JSON_INSTALL" OFF)

project(json)

set(JSON_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/json/json_config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/json/json.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/json/json_dump.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/json/json_binary.h
)

set(JSON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json_assert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json_dump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json_binary.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json_string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json_string.c
)

if(JSON_BINARY)
    set(JSON_HEADERS ${JSON_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/include/json/json_binary.h)
    set(JSON_SOURCES ${JSON_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/json_binary.c)
endif()

SOURCE_GROUP(json FILES ${JSON_HEADERS} ${JSON_SOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} STATIC ${JSON_SOURCES})

if(JSON_TEST)
    include(CTest)
    enable_testing()

    add_subdirectory(tests)
endif()

if(JSON_INSTALL)
  install(TARGETS json
          EXPORT json_targets
          LIBRARY DESTINATION "lib"
          ARCHIVE DESTINATION "lib"
          RUNTIME DESTINATION "bin"
          INCLUDES DESTINATION "include")

  install(FILES ${JSON_HEADERS}
          DESTINATION "include/json")
endif()

set(JSON_LIBRARIES json CACHE STRING "json libraries")