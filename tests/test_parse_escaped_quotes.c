#include "json/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    const char json[] = "{\"a\":\"x\\\"y\",\"b\":1,\"k\\\"q\":2}";

    js_allocator_t allocator;
    js_make_allocator_default( &test_alloc, &test_free, JS_NULLPTR, &allocator );

    js_element_t * document = JS_NULLPTR;

    /* Case #8: escaped quotes inside keys and values must not break parsing. */
    CHECK( js_parse( allocator, js_flag_none, json, sizeof( json ), &on_failed, JS_NULLPTR, &document ) == JS_SUCCESSFUL );

    js_element_t * b = js_object_get( document, "b" );
    CHECK( b != JS_NULLPTR );
    CHECK( js_get_integer( b ) == 1 );

    js_element_t * escaped_key = js_object_get( document, "k\\\"q" );
    CHECK( escaped_key != JS_NULLPTR );
    CHECK( js_get_integer( escaped_key ) == 2 );

    js_element_t * a = js_object_get( document, "a" );
    CHECK( a != JS_NULLPTR );
    CHECK( js_is_string( a ) == JS_TRUE );

    js_string_t value_a;
    js_get_string( a, &value_a );
    CHECK( value_a.size == 4 );
    CHECK( strncmp( value_a.value, "x\\\"y", value_a.size ) == 0 );

    js_free( document );

    return EXIT_SUCCESS;
}
