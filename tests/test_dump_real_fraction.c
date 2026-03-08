#include "json/json.h"
#include "json/json_dump.h"

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
    const char json[] = "{\"v\":0.25}";

    js_allocator_t allocator;
    js_make_allocator_default( &test_alloc, &test_free, JS_NULLPTR, &allocator );

    js_element_t * document = JS_NULLPTR;
    CHECK( js_parse( allocator, js_flag_none, json, sizeof( json ), &on_failed, JS_NULLPTR, &document ) == JS_SUCCESSFUL );

    char dump_memory[128] = {'\0'};
    js_buffer_t dump_buffer;
    js_make_buffer( dump_memory, sizeof( dump_memory ), &dump_buffer );

    js_dump_ctx_t dump_ctx;
    js_make_dump_ctx_buffer( &dump_buffer, &dump_ctx );

    CHECK( js_dump( document, &dump_ctx ) == JS_SUCCESSFUL );

    /* Case #5: integer part zero must be emitted as 0, not '-'. */
    CHECK( strcmp( dump_memory, "{\"v\":0.25}" ) == 0 );

    js_free( document );

    return EXIT_SUCCESS;
}
