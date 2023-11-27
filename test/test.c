#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

//////////////////////////////////////////////////////////////////////////
static void * __alloc( size_t size, void * ud )
{
    (void)ud;

    void * p = malloc( size );

    return p;
}
//////////////////////////////////////////////////////////////////////////
static void __free( void * ptr, void * ud )
{
    (void)ud;

    free( ptr );
}
//////////////////////////////////////////////////////////////////////////
typedef struct js_print_ctx_t
{
    int tab;
} js_print_ctx_t;
//////////////////////////////////////////////////////////////////////////
static void __js_array_print( size_t _index, const js_element_t * _element, void * _ud );
static void __js_object_print( size_t _index, const char * _key, size_t _size, const js_element_t * _element, void * _ud );
//////////////////////////////////////////////////////////////////////////
static void __js_array_print( size_t _index, const js_element_t * _element, void * _ud )
{
    js_print_ctx_t * ctx = (js_print_ctx_t *)_ud;

    if( _index != 0 )
    {
        printf( ", " );
    }

    js_type_e type = js_type( _element );

    switch( type )
    {
    case js_type_null:
        {
            printf( "null" );
        }break;
    case js_type_boolean:
        {
            js_bool_t b = js_get_boolean( _element );

            printf( "%s", b ? "true" : "false" );
        }break;
    case js_type_integer:
        {
            int64_t i = js_get_integer( _element );

            printf( "%lld", i );
        }break;
    case js_type_real:
        {
            double r = js_get_real( _element );

            printf( "%lf", r );
        }break;
    case js_type_string:
        {
            const char * s;
            js_size_t z;
            js_get_string( _element, &s, &z );

            printf( "\"%.*s\"", z, s );
        }break;
    case js_type_array:
        {
            printf( "[" );

            ctx->tab += 1;

            js_array_foreach( _element, &__js_array_print, ctx );

            ctx->tab -= 1;

            printf( "]" );
        }break;
    case js_type_object:
        {
            printf( "{\n" );

            ctx->tab += 1;

            js_object_foreach( _element, &__js_object_print, ctx );

            ctx->tab -= 1;

            printf( "}" );
        }break;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __js_object_print( size_t _index, const char * _key, size_t _size, const js_element_t * _element, void * _ud )
{
    js_print_ctx_t * ctx = (js_print_ctx_t *)_ud;

    for( int i = 0; i != ctx->tab; ++i )
    {
        printf( "\t" );
    }

    if( _index != 0 )
    {
        printf( ", " );
    }

    printf( "\"%.*s\": ", _size, _key );

    js_type_e type = js_type( _element );

    switch( type )
    {
    case js_type_null:
        {
            printf( "null" );
        }break;
    case js_type_boolean:
        {
            js_bool_t b = js_get_boolean( _element );

            printf( "%s", b ? "true" : "false" );
        }break;
    case js_type_integer:
        {
            int64_t i = js_get_integer( _element );

            printf( "%lld", i );
        }break;
    case js_type_real:
        {
            double r = js_get_real( _element );

            printf( "%lf", r );
        }break;
    case js_type_string:
        {
            const char * s;
            js_size_t z;
            js_get_string( _element, &s, &z );

            printf( "\"%.*s\"", z, s );
        }break;
    case js_type_array:
        {
            printf( "[" );

            ctx->tab += 1;

            js_array_foreach( _element, &__js_array_print, ctx );

            ctx->tab -= 1;

            printf( "]" );
        }break;
    case js_type_object:
        {
            printf( "{\n" );

            ctx->tab += 1;

            js_object_foreach( _element, &__js_object_print, ctx );

            ctx->tab -= 1;

            printf( "}" );
        }break;
    }
}

int main(int argc, char *argv[])
{
    char buff[] = "{\"name\":1.34,\"age\":18,\"floor\":[1,2.3,\"male\"], \"test\":[], \"blood\":{}, \"food\":[{\n}]}";

    js_allocator_t allocator;
    allocator.alloc = &__alloc;
    allocator.free = &__free;

    js_element_t * obj;
    if( js_parse( &allocator, buff, sizeof( buff ), &obj ) == JS_FAILURE )
    {
        return EXIT_FAILURE;
    }

    const js_element_t * el_age = js_object_get( obj, "age" );

    int64_t l = js_get_integer( el_age );

    printf( "age: %lld\n", l );

    js_print_ctx_t ctx;
    ctx.tab = 0;

    printf( "{" );
    js_object_foreach( obj, __js_object_print, &ctx );
    printf( "}" );

    printf( "\n" );

    return EXIT_SUCCESS;
}