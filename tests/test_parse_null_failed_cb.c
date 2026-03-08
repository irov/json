#include "json/json.h"

#include <stdio.h>
#include <stdlib.h>

static void * test_alloc( js_size_t _size, void * _ud )
{
    JS_UNUSED( _ud );

    return malloc( _size );
}

static void test_free( void * _ptr, void * _ud )
{
    JS_UNUSED( _ud );

    free( _ptr );
}

#define CHECK(EXPR) \
    do \
    { \
        if( !(EXPR) ) \
        { \
            printf( "CHECK FAILED: %s\n", #EXPR ); \
            return EXIT_FAILURE; \
        } \
    } while(0)

int main( void )
{
    const char invalid_json[] = "{\"a\":1";

    js_allocator_t allocator;
    js_make_allocator_default( &test_alloc, &test_free, JS_NULLPTR, &allocator );

    js_element_t * document = JS_NULLPTR;

    /* Case #4: parser should handle NULL failed callback safely. */
    js_result_t result = js_parse(
        allocator,
        js_flag_none,
        invalid_json,
        sizeof( invalid_json ),
        JS_NULLPTR,
        JS_NULLPTR,
        &document
    );

    CHECK( result == JS_FAILURE );
    CHECK( document == JS_NULLPTR );

    return EXIT_SUCCESS;
}
