#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>


//////////////////////////////////////////////////////////////////////////
typedef struct js_stats_t
{
    size_t memory_allocated_size;
    size_t memory_allocated_count;
    size_t memory_allocated_peak;
} js_stats_t;
//////////////////////////////////////////////////////////////////////////
static void * __alloc( size_t size, void * ud )
{
    js_stats_t * stats = (js_stats_t *)ud;

    void * p = malloc( size );

    stats->memory_allocated_size += size;
    stats->memory_allocated_count += 1;
    stats->memory_allocated_peak = stats->memory_allocated_size > stats->memory_allocated_peak
        ? stats->memory_allocated_size
        : stats->memory_allocated_peak;

    return p;
}
//////////////////////////////////////////////////////////////////////////
static void __free( void * ptr, void * ud )
{
    js_stats_t * stats = (js_stats_t *)ud;

    size_t size = _msize( ptr );

    stats->memory_allocated_size -= size;

    free( ptr );
}
//////////////////////////////////////////////////////////////////////////
static void __failed( const char * _data, const char * _end, const char * _message, void * _ud )
{
    (void)_data;
    (void)_message;
    (void)_ud;

    printf( "failed: %s\n", _message );
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
    char json[] = "{\"name\":1.34,\"age\":18,\"age1\":19,\"floor\":[1,-1,255,-255,256,-256,0,1,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,0,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,0,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,0,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,0,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,\"sdasdasda\",1,true,false,true,null,false,true,0.0,1.0,true  \n,\nfalse\n, null,  2.3,\"male\"], \"test\":[], \"blood\":{}, \"food\":[{\n}]}";

    //js_allocator_t allocator;
    //allocator.alloc = &__alloc;
    //allocator.free = &__free;
    //allocator.ud = JS_NULLPTR;

    js_stats_t stats;
    stats.memory_allocated_count = 0;
    stats.memory_allocated_size = 0;
    stats.memory_allocated_peak = 0;

    char memory[2048];

    js_buffer_t buff;
    js_make_buffer( memory, sizeof( memory ), &buff );

    js_allocator_t allocator;
    //js_make_allocator_buffer( &buff, &allocator );
    js_make_allocator_default( &__alloc, &__free, &stats, &allocator );

    js_element_t * obj;
    if( js_parse( allocator, js_flag_none | js_flag_node_pool, json, sizeof( json ), &__failed, JS_NULLPTR, &obj ) == JS_FAILURE )
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

    js_free( obj );

    printf( "json size: %zu\n"
        , sizeof( json )
    );

    printf( "memory leak: %zu count: %zu peak: %zu\n"
        , stats.memory_allocated_size
        , stats.memory_allocated_count
        , stats.memory_allocated_peak
    );

    return EXIT_SUCCESS;
}