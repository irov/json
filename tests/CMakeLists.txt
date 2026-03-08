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

    ADD_TEST(NAME ${testname} COMMAND ${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

ADD_JSON_TEST(load)
ADD_JSON_TEST(parse_invalid)
ADD_JSON_TEST(parse_types)
ADD_JSON_TEST(clone_independent)
ADD_JSON_TEST(dump_buffer_limits)
ADD_JSON_TEST(binary)
ADD_JSON_TEST(array_ops)
ADD_JSON_TEST(object_get_exact)
ADD_JSON_TEST(parse_escaped_quotes)
ADD_JSON_TEST(parse_truncated_array)
ADD_JSON_TEST(parse_null_failed_cb)
ADD_JSON_TEST(dump_real_fraction)
ADD_JSON_TEST(dump_int64_min)