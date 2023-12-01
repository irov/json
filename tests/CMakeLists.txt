macro(ADD_JSON_TEST testname)
    project(json_test_${testname})
    
    set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test_${testname}.c
    )
    
    include_directories(${JSON_SOURCE_DIR})
    
    ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})
    
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} json)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        FOLDER tests
    )

    ADD_TEST(NAME ${testname} COMMAND ./${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

ADD_JSON_TEST(load)