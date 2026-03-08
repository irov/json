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
    const char json[] = "{\"abc\":1}";

    js_allocator_t allocator;
    js_make_allocator_default( &test_alloc, &test_free, JS_NULLPTR, &allocator );

    js_element_t * document = JS_NULLPTR;

    CHECK( js_parse( allocator, js_flag_none, json, sizeof( json ), &on_failed, JS_NULLPTR, &document ) == JS_SUCCESSFUL );

    /* Case #7: js_object_get must match full key, not prefix only. */
    CHECK( js_object_get( document, "abc" ) != JS_NULLPTR );
    CHECK( js_object_get( document, "abcd" ) == JS_NULLPTR );

    js_free( document );

    return EXIT_SUCCESS;
}
