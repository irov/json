#ifndef JSON_H_
#define JSON_H_

#include "json/json_config.h"

typedef void * (*js_alloc_fun_t)(size_t _size, void * _ud);
typedef void (*js_free_fun_t)(void * _ptr, void * _ud);

typedef struct js_allocator_t
{
    js_alloc_fun_t alloc;
    js_free_fun_t free;
    void * ud;
} js_allocator_t;

void js_make_buffer( void * _memory, js_size_t _capacity, js_buffer_t * const _buffer );
js_size_t js_get_buffer_size( const js_buffer_t * _buffer );
js_size_t js_get_buffer_capacity( const js_buffer_t * _buffer );
js_size_t js_get_buffer_available( const js_buffer_t * _buffer );
void js_rewind_buffer( js_buffer_t * _buffer );

void js_make_allocator_buffer( js_buffer_t * _buffer, js_allocator_t * const _allocator );
void js_make_allocator_default( js_alloc_fun_t _alloc, js_free_fun_t _free, void * _ud, js_allocator_t * const _allocator );

typedef void (*js_failed_fun_t)(const char * _pointer, const char * _end, const char * _message, void * _ud);

js_result_t js_parse( js_allocator_t _allocator, js_flags_t _flags, const char * _data, js_size_t _size, js_failed_fun_t _failed, void * _ud, js_element_t ** _documet );
js_result_t js_clone( js_allocator_t _allocator, js_flags_t _flags, const js_element_t * _base, js_element_t ** _total );
js_result_t js_patch( js_allocator_t _allocator, js_flags_t _flags, const js_element_t * _base, const js_element_t * _patch, js_element_t ** _total );
js_result_t js_create( js_allocator_t _allocator, js_flags_t _flags, js_element_t ** _documet );

js_result_t js_object_add_field_null( js_element_t * _documet, js_element_t * _element, js_string_t _key );
js_result_t js_object_add_field_true( js_element_t * _documet, js_element_t * _element, js_string_t _key );
js_result_t js_object_add_field_false( js_element_t * _documet, js_element_t * _element, js_string_t _key );
js_result_t js_object_add_field_boolean( js_element_t * _documet, js_element_t * _element, js_string_t _key, js_bool_t _value );
js_result_t js_object_add_field_integer( js_element_t * _documet, js_element_t * _element, js_string_t _key, js_integer_t _value );
js_result_t js_object_add_field_real( js_element_t * _documet, js_element_t * _element, js_string_t _key, js_real_t _value );
js_result_t js_object_add_field_string( js_element_t * _documet, js_element_t * _element, js_string_t _key, const char * _value );
js_result_t js_object_add_field_stringn( js_element_t * _documet, js_element_t * _element, js_string_t _key, js_string_t _value );
js_result_t js_object_add_field_array( js_element_t * _documet, js_element_t * _element, js_string_t _key, js_element_t ** _array );
js_result_t js_object_add_field_object( js_element_t * _documet, js_element_t * _element, js_string_t _key, js_element_t ** _object );

js_result_t js_array_push_null( js_element_t * _documet, js_element_t * _element );
js_result_t js_array_push_true( js_element_t * _documet, js_element_t * _element );
js_result_t js_array_push_false( js_element_t * _documet, js_element_t * _element );
js_result_t js_array_push_boolean( js_element_t * _documet, js_element_t * _element, js_bool_t _value );
js_result_t js_array_push_integer( js_element_t * _documet, js_element_t * _element, js_integer_t _value );
js_result_t js_array_push_real( js_element_t * _documet, js_element_t * _element, js_real_t _value );
js_result_t js_array_push_string( js_element_t * _documet, js_element_t * _element, const char * _value );
js_result_t js_array_push_stringn( js_element_t * _documet, js_element_t * _element, js_string_t _value );
js_result_t js_array_push_array( js_element_t * _documet, js_element_t * _element, js_element_t ** _array );
js_result_t js_array_push_object( js_element_t * _documet, js_element_t * _element, js_element_t ** _object );

void js_array_remove( js_element_t * _document, js_element_t * _element, js_size_t _index );
void js_array_clear( js_element_t * _document, js_element_t * _element );

void js_free( js_element_t * _element );

js_type_t js_type( const js_element_t * _element );

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
js_integer_t js_get_integer( const js_element_t * _element );
js_real_t js_get_real( const js_element_t * _element );
void js_get_string( const js_element_t * _element, js_string_t * const _value );

js_size_t js_array_size( const js_element_t * _element );
js_element_t * js_array_get( const js_element_t * _element, js_size_t _index );

js_size_t js_object_size( const js_element_t * _element );
js_element_t * js_object_get( const js_element_t * _element, const char * _key );
js_element_t * js_object_getn( const js_element_t * _element, js_string_t _key );

typedef js_result_t( *js_array_visitor_fun_t )(js_size_t _index, const js_element_t * _value, void * _ud);
js_result_t js_array_visit( const js_element_t * _element, js_array_visitor_fun_t _visitor, void * _ud );

typedef js_result_t( *js_object_visitor_fun_t )(js_size_t _index, const js_element_t * _key, const js_element_t * _value, void * _ud);
js_result_t js_object_visit( const js_element_t * _element, js_object_visitor_fun_t _visitor, void * _ud );

typedef void(*js_array_foreach_fun_t)(js_size_t _index, const js_element_t * _value, void * _ud);
void js_array_foreach( const js_element_t * _element, js_array_foreach_fun_t _foreach, void * _ud );

typedef void(*js_object_foreach_fun_t)(js_size_t _index, const js_element_t * _key, const js_element_t * _value, void * _ud);
void js_object_foreach( const js_element_t * _element, js_object_foreach_fun_t _foreach, void * _ud );

#endif
