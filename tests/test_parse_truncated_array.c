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

typedef struct failed_ctx_t
{
    int called;
} failed_ctx_t;

static void on_failed( const char * _pointer, const char * _end, const char * _message, void * _ud )
{
    JS_UNUSED( _pointer );
    JS_UNUSED( _end );
    JS_UNUSED( _message );

    failed_ctx_t * ctx = (failed_ctx_t *)_ud;
    ctx->called = 1;
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
    const char invalid_json[] = "{\"a\":[1";

    js_allocator_t allocator;
    js_make_allocator_default( &test_alloc, &test_free, JS_NULLPTR, &allocator );

    failed_ctx_t failed_ctx;
    failed_ctx.called = 0;

    js_element_t * document = JS_NULLPTR;

    /* Case #3: invalid truncated array should return failure, not crash. */
    js_result_t result = js_parse(
        allocator,
        js_flag_none,
        invalid_json,
        sizeof( invalid_json ),
        &on_failed,
        &failed_ctx,
        &document
    );

    CHECK( result == JS_FAILURE );
    CHECK( failed_ctx.called == 1 );
    CHECK( document == JS_NULLPTR );

    return EXIT_SUCCESS;
}
