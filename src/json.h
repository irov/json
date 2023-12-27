#ifndef JSON_H_
#define JSON_H_

#include <stddef.h>
#include <stdint.h>

typedef uint8_t js_result_t;
typedef uint8_t js_bool_t;
typedef size_t js_size_t;

typedef int64_t js_integer_value_t;
typedef double js_real_value_t;

#ifndef JS_SUCCESSFUL
#define JS_SUCCESSFUL ((js_result_t)0)
#endif

#ifndef JS_FAILURE
#define JS_FAILURE ((js_result_t)1)
#endif

#ifndef JS_FALSE
#define JS_FALSE ((js_bool_t)0)
#endif

#ifndef JS_TRUE
#define JS_TRUE ((js_bool_t)1)
#endif

#ifndef JS_UNUSED
#define JS_UNUSED(X) (void)(X)
#endif

#ifndef JS_NULLPTR
#define JS_NULLPTR ((void*)0)
#endif

#ifndef JS_ALLOCATOR_MEMORY_CHECK_ENABLE
#   ifndef NDEBUG
#       define JS_ALLOCATOR_MEMORY_CHECK_ENABLE 1
#   else
#       define JS_ALLOCATOR_MEMORY_CHECK_ENABLE 0
#   endif
#endif

typedef enum js_type_e
{
    js_type_null = 0,
    js_type_false = 1 << 1,
    js_type_true = 1 << 2,
    js_type_integer = 1 << 3,
    js_type_real = 1 << 4,
    js_type_string = 1 << 5,
    js_type_array = 1 << 6,
    js_type_object = 1 << 7,
} js_type_e;

typedef enum js_flags_e
{
    js_flag_none = 0,
    js_flag_string_inplace = 1 << 0,
    js_flag_node_pool = 1 << 1,
} js_flags_e;

typedef struct js_element_t js_element_t;

typedef void * (*js_alloc_fun_t)(size_t _size, void * _ud);
typedef void (*js_free_fun_t)(void * _ptr, void * _ud);

typedef struct js_allocator_t
{
    js_alloc_fun_t alloc;
    js_free_fun_t free;
    void * ud;
} js_allocator_t;

typedef struct js_buffer_t
{
    uint8_t * begin;
    uint8_t * end;
    uint8_t * memory;    
} js_buffer_t;

void js_make_buffer( void * _memory, js_size_t _capacity, js_buffer_t * const _buffer );
js_size_t js_get_buffer_size( const js_buffer_t * _buffer );
js_size_t js_get_buffer_capacity( const js_buffer_t * _buffer );
js_size_t js_get_buffer_available( const js_buffer_t * _buffer );
void js_rewind_buffer( js_buffer_t * _buffer );

void js_make_allocator_buffer( js_buffer_t * _buffer, js_allocator_t * const _allocator );
void js_make_allocator_default( js_alloc_fun_t _alloc, js_free_fun_t _free, void * ud, js_allocator_t * const _allocator );

typedef void (*js_failed_fun_t)(const char * _pointer, const char * _end, const char * _message, void * _ud);

js_result_t js_parse( js_allocator_t _allocator, js_flags_e _flags, const char * _data, js_size_t _size, js_failed_fun_t _failed, void * _ud, js_element_t ** _element );
js_result_t js_clone( js_allocator_t _allocator, js_flags_e _flags, const js_element_t * _base, js_element_t ** _total );
js_result_t js_patch( js_allocator_t _allocator, js_flags_e _flags, const js_element_t * _base, const js_element_t * _patch, js_element_t ** _total );

void js_free( js_element_t * _element );

js_type_e js_type( const js_element_t * _element );

js_bool_t js_is_null( const js_element_t * _element );
js_bool_t js_is_false( const js_element_t * _element );
js_bool_t js_is_true( const js_element_t * _element );
js_bool_t js_is_boolean( const js_element_t * _element );
js_bool_t js_is_integer( const js_element_t * _element );
js_bool_t js_is_real( const js_element_t * _element );
js_bool_t js_is_string( const js_element_t * _element );
js_bool_t js_is_array( const js_element_t * _element );
js_bool_t js_is_object( const js_element_t * _element );

js_bool_t js_get_boolean( const js_element_t * _element );
js_integer_value_t js_get_integer( const js_element_t * _element );
js_real_value_t js_get_real( const js_element_t * _element );
void js_get_string( const js_element_t * _element, const char ** _value, js_size_t * const _size );

js_size_t js_array_size( const js_element_t * _element );
const js_element_t * js_array_get( const js_element_t * _element, js_size_t _index );

js_size_t js_object_size( const js_element_t * _element );
const js_element_t * js_object_get( const js_element_t * _element, const char * _key );
const js_element_t * js_object_getn( const js_element_t * _element, const char * _key, js_size_t _size );

typedef js_result_t( *js_array_visitor_fun_t )(js_size_t _index, const js_element_t * _value, void * _ud);
js_result_t js_array_visit( const js_element_t * _element, js_array_visitor_fun_t _visitor, void * _ud );

typedef js_result_t( *js_object_visitor_fun_t )(js_size_t _index, const char * _key, js_size_t _size, const js_element_t * _value, void * _ud);
js_result_t js_object_visit( const js_element_t * _element, js_object_visitor_fun_t _visitor, void * _ud );

typedef void(*js_array_foreach_fun_t)(js_size_t _index, const js_element_t * _value, void * _ud);
void js_array_foreach( const js_element_t * _element, js_array_foreach_fun_t _foreach, void * _ud );

typedef void(*js_object_foreach_fun_t)(js_size_t _index, const char * _key, js_size_t _size, const js_element_t * _value, void * _ud);
void js_object_foreach( const js_element_t * _element, js_object_foreach_fun_t _foreach, void * _ud );

typedef char * (*js_dump_buffer_fun_t)(js_size_t _size, void * _ud);

typedef struct js_dump_ctx_t
{   
    js_dump_buffer_fun_t buffer;
    void * ud;
} js_dump_ctx_t;

void js_make_dump_ctx_buffer( js_buffer_t * _buffer, js_dump_ctx_t * const _ctx );

js_result_t js_dump( const js_element_t * _element, js_dump_ctx_t * _ctx );

#endif