#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>


//////////////////////////////////////////////////////////////////////////
typedef struct js_stats_t
{
    js_size_t memory_allocated_size;
    js_size_t memory_allocated_count;
    js_size_t memory_allocated_peak;
} js_stats_t;
//////////////////////////////////////////////////////////////////////////
static void * __alloc( js_size_t size, void * ud )
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
static void __free( js_size_t * ptr, void * ud )
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
static char * __js_dump_buffer( js_size_t _size, void * _ud )
{
    js_buffer_t * buffer = (js_buffer_t *)_ud;

    if( buffer->memory + _size > buffer->end )
    {
        buffer->memory = buffer->end;

        return JS_NULLPTR;
    }

    uint8_t * new_buffer = buffer->memory;

    buffer->memory += _size;
    
    return (char *)new_buffer;
}
//////////////////////////////////////////////////////////////////////////
static void __js_print( const js_element_t * _element )
{
    char dump_memory[2048];

    js_buffer_t dump_buff;
    js_make_buffer( dump_memory, sizeof( dump_memory ), &dump_buff );

    js_dump_ctx_t dump_ctx;
    dump_ctx.buffer = &__js_dump_buffer;
    dump_ctx.ud = &dump_buff;

    js_dump( _element, &dump_ctx );

    printf( "%s", dump_memory );
}
//////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    char json_base[] = "{\"name\":1.34,\"age\":18,\"age1\":19,\"floor\":[1,-1,255,-255,256,-256,0,1,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,0,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,0,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,0,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,0,1,2,3,4,5,6,7,8,9,10,11,12,23,34,56,76,\"sdasdasda\",1,true,false,true,null,false,true,0.0,1.0,true  \n,\nfalse\n, null,  2.3,\"male\"], \"test\":   \n[], \"blood\":{}, \"food\":[{\n}]}";

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

    js_element_t * base;
    if( js_parse( allocator, js_flag_none | js_flag_node_pool, json_base, sizeof( json_base ), &__failed, JS_NULLPTR, &base ) == JS_FAILURE )
    {
        return EXIT_FAILURE;
    }

    const js_element_t * el_age = js_object_get( base, "age" );

    int64_t l = js_get_integer( el_age );

    printf( "age: %lld\n", l );

    printf( "base: " );
    __js_print( base );
    printf( "\n" );

    char json_patch[] = "{\"age\":19,\"floor\":null}";

    js_element_t * patch;
    if( js_parse( allocator, js_flag_none | js_flag_node_pool, json_patch, sizeof( json_patch ), &__failed, JS_NULLPTR, &patch ) == JS_FAILURE )
    {
        return EXIT_FAILURE;
    }

    printf( "patch: " );
    __js_print( patch );
    printf( "\n" );

    js_element_t * total;
    if( js_patch( allocator, js_flag_none | js_flag_node_pool, base, patch, &total ) == JS_FAILURE )
    {
        return EXIT_FAILURE;
    }

    printf( "total: " );
    __js_print( total );
    printf( "\n" );

    js_free( base );
    js_free( patch );
    js_free( total );

    printf( "json size: %zu\n"
        , sizeof( json_base )
    );

    printf( "memory leak: %zu count: %zu peak: %zu\n"
        , stats.memory_allocated_size
        , stats.memory_allocated_count
        , stats.memory_allocated_peak
    );

    return EXIT_SUCCESS;
}