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
    js_allocator_t allocator;
    js_make_allocator_default( &test_alloc, &test_free, JS_NULLPTR, &allocator );

    js_element_t * document = JS_NULLPTR;
    CHECK( js_create( allocator, js_flag_none, &document ) == JS_SUCCESSFUL );

    const js_integer_t min_int64 = (-9223372036854775807LL - 1LL);
    js_string_t key = JS_CONST_STRING( "v" );

    CHECK( js_object_add_field_integer( document, document, key, min_int64 ) == JS_SUCCESSFUL );

    char dump_memory[128] = {'\0'};
    js_buffer_t dump_buffer;
    js_make_buffer( dump_memory, sizeof( dump_memory ), &dump_buffer );

    js_dump_ctx_t dump_ctx;
    js_make_dump_ctx_buffer( &dump_buffer, &dump_ctx );

    CHECK( js_dump( document, &dump_ctx ) == JS_SUCCESSFUL );

    /* Case #6: dumping INT64_MIN must be stable and exact. */
    CHECK( strcmp( dump_memory, "{\"v\":-9223372036854775808}" ) == 0 );

    js_free( document );

    return EXIT_SUCCESS;
}
