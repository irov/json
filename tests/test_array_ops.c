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

static void on_failed( const char * _pointer, const char * _end, const char * _message, void * _ud )
{
    JS_UNUSED( _pointer );
    JS_UNUSED( _end );
    JS_UNUSED( _ud );

    printf( "parse failed: %s\n", _message );
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
    const char json[] = "{\"arr\":[1,2,3]}";

    js_allocator_t allocator;
    js_make_allocator_default( &test_alloc, &test_free, JS_NULLPTR, &allocator );

    js_element_t * document = JS_NULLPTR;

    CHECK( js_parse( allocator, js_flag_none, json, sizeof( json ), &on_failed, JS_NULLPTR, &document ) == JS_SUCCESSFUL );

    js_element_t * arr = js_object_get( document, "arr" );
    CHECK( arr != JS_NULLPTR );
    CHECK( js_is_array( arr ) == JS_TRUE );
    CHECK( js_array_size( arr ) == 3 );

    /* Case #2: remove head safely and keep list valid. */
    js_array_remove( document, arr, 0 );
    CHECK( js_array_size( arr ) == 2 );
    CHECK( js_get_integer( js_array_get( arr, 0 ) ) == 2 );
    CHECK( js_get_integer( js_array_get( arr, 1 ) ) == 1 );

    js_array_remove( document, arr, 1 );
    CHECK( js_array_size( arr ) == 1 );
    CHECK( js_get_integer( js_array_get( arr, 0 ) ) == 2 );

    /* Case #1: clear array must not use freed node pointers. */
    js_array_clear( document, arr );
    CHECK( js_array_size( arr ) == 0 );

    CHECK( js_array_push_integer( document, arr, 42 ) == JS_SUCCESSFUL );
    CHECK( js_array_size( arr ) == 1 );
    CHECK( js_get_integer( js_array_get( arr, 0 ) ) == 42 );

    js_free( document );

    return EXIT_SUCCESS;
}
