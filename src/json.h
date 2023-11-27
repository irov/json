#ifndef JSON_H_
#define JSON_H_

#include <stdint.h>

typedef uint8_t js_result_t;
typedef uint8_t js_bool_t;
typedef size_t js_size_t;

static const js_result_t JS_SUCCESSFUL = 0;
static const js_result_t JS_FAILURE = 1;

static const js_bool_t JS_FALSE = 0;
static const js_bool_t JS_TRUE = 1;

#ifndef JS_UNUSED
#define JS_UNUSED(X) (void)(X)
#endif

#ifndef JS_NULLPTR
#define JS_NULLPTR ((void*)0)
#endif

typedef enum js_type_e
{
    js_type_null,
    js_type_boolean,
    js_type_integer,
    js_type_real,
    js_type_string,
    js_type_array,
    js_type_object,
} js_type_e;

typedef struct js_allocator_t
{
    void * (*alloc)(size_t size, void * ud);
    void (*free)(void * ptr, void * ud);
    void * ud;
} js_allocator_t;

typedef struct js_element_t js_element_t;

js_result_t js_parse( js_allocator_t * _allocator, const void * _data, js_size_t _size, js_element_t ** _element );

js_type_e js_type( const js_element_t * _element );

js_bool_t js_is_null( const js_element_t * _element );
js_bool_t js_is_boolean( const js_element_t * _element );
js_bool_t js_is_integer( const js_element_t * _element );
js_bool_t js_is_real( const js_element_t * _element );
js_bool_t js_is_string( const js_element_t * _element );
js_bool_t js_is_array( const js_element_t * _element );
js_bool_t js_is_object( const js_element_t * _element );

js_bool_t js_get_boolean( const js_element_t * _element );
int64_t js_get_integer( const js_element_t * _element );
double js_get_real( const js_element_t * _element );
void js_get_string( const js_element_t * _element, const char ** _value, js_size_t * const _size );

js_size_t js_array_size( const js_element_t * _element );
const js_element_t * js_array_get( const js_element_t * _element, js_size_t _index );

js_size_t js_object_size( const js_element_t * _element );
const js_element_t * js_object_get( const js_element_t * _element, const char * _key );
const js_element_t * js_object_getn( const js_element_t * _element, const char * _key, js_size_t _size );

typedef void (*js_object_foreach_t)(size_t _index, const char * _key, size_t _size, const js_element_t * _value, void * _ud);
void js_object_foreach( const js_element_t * _element, js_object_foreach_t _foreach, void * _ud );

typedef void (*js_array_foreach_t)(size_t _index, const js_element_t * _value, void * _ud);
void js_array_foreach( const js_element_t * _element, js_array_foreach_t _foreach, void * _ud );

#endif