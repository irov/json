#include "json.h"

#include <stdlib.h>

#ifndef JS_NODEBLOCK_SIZE
#define JS_NODEBLOCK_SIZE 64
#endif

#if JS_ALLOCATOR_MEMORY_CHECK_ENABLE
#   define JS_ALLOCATOR_MEMORY_CHECK(Ptr, Ret) if( (Ptr) == JS_NULLPTR ) return (Ret);
#   define JS_ALLOCATOR_MEMORY_CHECK(Ptr, Ret) if( (Ptr) == JS_NULLPTR ) return (Ret);
#else
#   define JS_ALLOCATOR_MEMORY_CHECK(Ptr, Ret)
#   define JS_ALLOCATOR_MEMORY_CHECK(Ptr, Ret)
#endif

#define JS_ALLOCATOR_NEW(Allocator, Type) ((Type *)(Allocator)->alloc( sizeof( Type ), (Allocator)->ud ))
#define JS_ALLOCATOR_NEW_EX(Allocator, Type, ExSize) ((Type *)(Allocator)->alloc( sizeof( Type ) + (ExSize), (Allocator)->ud ))

//////////////////////////////////////////////////////////////////////////
typedef struct js_element_t
{
    js_type_e type;
} js_element_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_null_t
{
    js_element_t base;
} js_null_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_boolean_t
{
    js_element_t base;
    js_bool_t value;
} js_boolean_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_integer_t
{
    js_element_t base;
    js_integer_value_t value;
} js_integer_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_real_t
{
    js_element_t base;
    js_real_value_t value;
} js_real_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_string_t
{
    js_element_t base;
    const char * value;
    js_size_t size;
} js_string_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_string_buffer_t
{
    js_string_t base;

    char buffer[];
} js_string_buffer_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_node_t
{
    js_element_t * element;

    struct js_node_t * next;
} js_node_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_block_t
{
    js_node_t nodes[JS_NODEBLOCK_SIZE];

    struct js_block_t * prev;
} js_block_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_object_t
{
    js_element_t base;
    js_size_t size;
    js_node_t * keys;
    js_node_t * values;
} js_object_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_array_t
{
    js_element_t base;
    js_size_t size;
    js_node_t * values;
} js_array_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_document_t
{
    js_object_t object;
    js_allocator_t allocator;
    js_flags_e flags;

    js_node_t * (*node_create)(struct js_document_t * _document, js_element_t * _element);
    void (*node_destroy)(struct js_document_t * _document, js_node_t * _node);
    js_string_t * (*string_create)(js_allocator_t * _allocator, const char * _value, js_size_t _size);
    void (*string_destroy)(js_allocator_t * _allocator, js_string_t * _string);

    js_node_t * free_node;
    js_block_t * free_block;
} js_document_t;
//////////////////////////////////////////////////////////////////////////
static js_allocator_t * __js_document_allocator( js_document_t * _document )
{
    js_allocator_t * allocator = &_document->allocator;

    return allocator;
}
//////////////////////////////////////////////////////////////////////////
static js_null_t * __js_null_create( js_allocator_t * _allocator )
{
    JS_UNUSED( _allocator );

    static js_null_t cache_null = {js_type_null};

    js_null_t * null = &cache_null;

    return null;
}
//////////////////////////////////////////////////////////////////////////
static js_boolean_t * __js_boolean_create( js_allocator_t * _allocator, js_bool_t _value )
{
    JS_UNUSED( _allocator );

    static js_boolean_t cache_booleans[2] = {{js_type_boolean, JS_FALSE}, {js_type_boolean, JS_TRUE}};

    js_boolean_t * boolean = cache_booleans + _value;

    return boolean;
}
//////////////////////////////////////////////////////////////////////////
static js_integer_t * __js_integer_create( js_allocator_t * _allocator, js_integer_value_t _value )
{
#define __JS_DECLARE_POSINT( N ) { \
    js_type_integer, N * 16 + 0}, \
    {js_type_integer, N * 16 + 1}, \
    {js_type_integer, N * 16 + 2}, \
    {js_type_integer, N * 16 + 3}, \
    {js_type_integer, N * 16 + 4}, \
    {js_type_integer, N * 16 + 5}, \
    {js_type_integer, N * 16 + 6}, \
    {js_type_integer, N * 16 + 7}, \
    {js_type_integer, N * 16 + 8}, \
    {js_type_integer, N * 16 + 9}, \
    {js_type_integer, N * 16 + 10}, \
    {js_type_integer, N * 16 + 11}, \
    {js_type_integer, N * 16 + 12}, \
    {js_type_integer, N * 16 + 13}, \
    {js_type_integer, N * 16 + 14}, \
    {js_type_integer, N * 16 + 15}

#define __JS_DECLARE_NEGINT( N ) { \
    js_type_integer, - N * 16 - 0}, \
    {js_type_integer, - N * 16 - 1}, \
    {js_type_integer, - N * 16 - 2}, \
    {js_type_integer, - N * 16 - 3}, \
    {js_type_integer, - N * 16 - 4}, \
    {js_type_integer, - N * 16 - 5}, \
    {js_type_integer, - N * 16 - 6}, \
    {js_type_integer, - N * 16 - 7}, \
    {js_type_integer, - N * 16 - 8}, \
    {js_type_integer, - N * 16 - 9}, \
    {js_type_integer, - N * 16 - 10}, \
    {js_type_integer, - N * 16 - 11}, \
    {js_type_integer, - N * 16 - 12}, \
    {js_type_integer, - N * 16 - 13}, \
    {js_type_integer, - N * 16 - 14}, \
    {js_type_integer, - N * 16 - 15}

    if( _value >= 0 && _value < 256 )
    {
        static js_integer_t cache_positive_integers[256] = {
            __JS_DECLARE_POSINT( 0 ), __JS_DECLARE_POSINT( 1 ), __JS_DECLARE_POSINT( 2 ), __JS_DECLARE_POSINT( 3 ),
            __JS_DECLARE_POSINT( 4 ), __JS_DECLARE_POSINT( 5 ), __JS_DECLARE_POSINT( 6 ), __JS_DECLARE_POSINT( 7 ),
            __JS_DECLARE_POSINT( 8 ), __JS_DECLARE_POSINT( 9 ), __JS_DECLARE_POSINT( 10 ), __JS_DECLARE_POSINT( 11 ),
            __JS_DECLARE_POSINT( 12 ), __JS_DECLARE_POSINT( 13 ), __JS_DECLARE_POSINT( 14 ), __JS_DECLARE_POSINT( 15 )
        };

        js_integer_t * integer = cache_positive_integers + _value;

        return integer;
    }
    else if( _value < 0 && _value > -256 )
    {
        static js_integer_t cache_negative_integers[256] = {
            __JS_DECLARE_NEGINT( 0 ), __JS_DECLARE_NEGINT( 1 ), __JS_DECLARE_NEGINT( 2 ), __JS_DECLARE_NEGINT( 3 ),
            __JS_DECLARE_NEGINT( 4 ), __JS_DECLARE_NEGINT( 5 ), __JS_DECLARE_NEGINT( 6 ), __JS_DECLARE_NEGINT( 7 ),
            __JS_DECLARE_NEGINT( 8 ), __JS_DECLARE_NEGINT( 9 ), __JS_DECLARE_NEGINT( 10 ), __JS_DECLARE_NEGINT( 11 ),
            __JS_DECLARE_NEGINT( 12 ), __JS_DECLARE_NEGINT( 13 ), __JS_DECLARE_NEGINT( 14 ), __JS_DECLARE_NEGINT( 15 )
        };

        js_integer_t * integer = cache_negative_integers - _value;

        return integer;
    }

    js_integer_t * integer = JS_ALLOCATOR_NEW( _allocator, js_integer_t );

    JS_ALLOCATOR_MEMORY_CHECK( integer, JS_NULLPTR );

    integer->base.type = js_type_integer;

    integer->value = _value;

    return integer;
}
//////////////////////////////////////////////////////////////////////////
static js_real_t * __js_real_create( js_allocator_t * _allocator, js_real_value_t _value )
{
    if( _value == 0.0 )
    {
        static js_real_t cache_zero_real = {js_type_real, 0.0};

        js_real_t * real = &cache_zero_real;

        return real;
    }
    else if( _value == 1.0 )
    {
        static js_real_t cache_one_real = {js_type_real, 1.0};

        js_real_t * real = &cache_one_real;

        return real;
    }

    js_real_t * real = JS_ALLOCATOR_NEW( _allocator, js_real_t );

    JS_ALLOCATOR_MEMORY_CHECK( real, JS_NULLPTR );

    real->base.type = js_type_real;

    real->value = _value;

    return real;
}
//////////////////////////////////////////////////////////////////////////
static js_string_t * __js_string_create_allocator( js_allocator_t * _allocator, const char * _value, js_size_t _size )
{
    js_string_buffer_t * string_buffer = JS_ALLOCATOR_NEW_EX( _allocator, js_string_buffer_t, _size );

    JS_ALLOCATOR_MEMORY_CHECK( string_buffer, JS_NULLPTR );

    string_buffer->base.base.type = js_type_string;

    string_buffer->base.value = string_buffer->buffer;
    string_buffer->base.size = _size;

    for( js_size_t index = 0; index != _size; ++index )
    {
        string_buffer->buffer[index] = _value[index];
    }

    js_string_t * string = (js_string_t *)string_buffer;

    return string;
}
//////////////////////////////////////////////////////////////////////////
static js_string_t * __js_string_create_inplace( js_allocator_t * _allocator, const char * _value, js_size_t _size )
{
    js_string_t * string = JS_ALLOCATOR_NEW( _allocator, js_string_t );

    JS_ALLOCATOR_MEMORY_CHECK( string, JS_NULLPTR );

    string->base.type = js_type_string;

    string->value = _value;
    string->size = _size;

    return string;
}
//////////////////////////////////////////////////////////////////////////
static js_object_t * __js_object_create( js_allocator_t * _allocator )
{
    js_object_t * object = JS_ALLOCATOR_NEW( _allocator, js_object_t );

    JS_ALLOCATOR_MEMORY_CHECK( object, JS_NULLPTR );

    object->base.type = js_type_object;

    object->size = 0;
    object->keys = JS_NULLPTR;
    object->values = JS_NULLPTR;

    return object;
}
//////////////////////////////////////////////////////////////////////////
static js_array_t * __js_array_create( js_allocator_t * _allocator )
{
    js_array_t * array = JS_ALLOCATOR_NEW( _allocator, js_array_t );

    JS_ALLOCATOR_MEMORY_CHECK( array, JS_NULLPTR );

    array->base.type = js_type_array;

    array->size = 0;
    array->values = JS_NULLPTR;

    return array;
}
//////////////////////////////////////////////////////////////////////////
static js_block_t * __js_block_create( js_allocator_t * _allocator, js_node_t ** _free )
{
    js_block_t * block = JS_ALLOCATOR_NEW( _allocator, js_block_t );

    JS_ALLOCATOR_MEMORY_CHECK( block, JS_NULLPTR );

    js_node_t * free = JS_NULLPTR;

    for( js_size_t index = 0; index != JS_NODEBLOCK_SIZE; ++index )
    {
        js_node_t * node = block->nodes + index;

        node->element = JS_NULLPTR;
        node->next = free;

        free = node;
    }
    
    block->prev = JS_NULLPTR;

    *_free = free;

    return block;
}
//////////////////////////////////////////////////////////////////////////
static js_node_t * __js_node_create_allocator( js_document_t * _document, js_element_t * _element )
{
    js_allocator_t * allocator = __js_document_allocator( _document );

    js_node_t * node = JS_ALLOCATOR_NEW( allocator, js_node_t );

    JS_ALLOCATOR_MEMORY_CHECK( node, JS_NULLPTR );

    node->element = _element;
    node->next = JS_NULLPTR;

    return node;
}
//////////////////////////////////////////////////////////////////////////
static js_node_t * __js_node_create_from_pool( js_document_t * _document, js_element_t * _element )
{
    js_allocator_t * allocator = __js_document_allocator( _document );

    if( _document->free_node == JS_NULLPTR )
    {
        js_block_t * block = __js_block_create( allocator, &_document->free_node );

        JS_ALLOCATOR_MEMORY_CHECK( block, JS_NULLPTR );

        block->prev = _document->free_block;

        _document->free_block = block;
    }

    js_node_t * node = _document->free_node;
    _document->free_node = _document->free_node->next;

    node->element = _element;
    node->next = JS_NULLPTR;

    return node;
}
//////////////////////////////////////////////////////////////////////////
static void __js_string_destroy_inplace( js_allocator_t * _allocator, js_string_t * _string )
{
    _allocator->free( _string, _allocator->ud );
}
//////////////////////////////////////////////////////////////////////////
static void __js_string_destroy_allocator( js_allocator_t * _allocator, js_string_t * _string )
{
    _allocator->free( _string, _allocator->ud );
}
//////////////////////////////////////////////////////////////////////////
static void __js_element_destroy( js_document_t * _document, js_element_t * _element );
static void __js_array_destroy( js_document_t * _document, js_array_t * _array );
static void __js_object_destroy( js_document_t * _document, js_object_t * _object );
//////////////////////////////////////////////////////////////////////////
static void __js_element_destroy( js_document_t * _document, js_element_t * _element )
{
    js_allocator_t * allocator = __js_document_allocator( _document );

    js_type_e type = js_type( _element );

    switch( type )
    {
    case js_type_null:
    case js_type_boolean:
        {
            // cache
        }break;
    case js_type_integer:
        {
            js_integer_t * integer = (js_integer_t *)_element;

            js_integer_value_t value = integer->value;

            if( value > -256 && value < 256 )
            {
                // cache
            }
            else
            {
                allocator->free( integer, allocator->ud );
            }
        }break;
    case js_type_real:
        {
            js_real_t * real = (js_real_t *)_element;

            js_real_value_t value = real->value;

            if( value == 0.0 || value == 1.0 )
            {
                // cache
            }
            else
            {
                allocator->free( real, allocator->ud );
            }
        }break;
    case js_type_string:
        {
            js_string_t * string = (js_string_t *)_element;

            _document->string_destroy( allocator, string );
        }break;
    case js_type_array:
        {
            js_array_t * array = (js_array_t *)_element;

            __js_array_destroy( _document, array );
        }break;
    case js_type_object:
        {
            js_object_t * object = (js_object_t *)_element;

            __js_object_destroy( _document, object );
        }break;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __js_array_destroy( js_document_t * _document, js_array_t * _array )
{
    js_node_t * it_node = _array->values;

    for( ; it_node != JS_NULLPTR; )
    {
        js_node_t * free_node = it_node;

        it_node = it_node->next;

        _document->node_destroy( _document, free_node );
    }

    js_allocator_t * allocator = __js_document_allocator( _document );

    allocator->free( _array, allocator->ud );
}
//////////////////////////////////////////////////////////////////////////
static void __js_object_destroy( js_document_t * _document, js_object_t * _object )
{
    js_node_t * it_key = _object->keys;
    js_node_t * it_value = _object->values;

    for( ; it_key != JS_NULLPTR; )
    {
        js_node_t * free_key = it_key;
        js_node_t * free_value = it_value;

        it_key = it_key->next;
        it_value = it_value->next;

        _document->node_destroy( _document, free_key );
        _document->node_destroy( _document, free_value );
    }

    js_allocator_t * allocator = __js_document_allocator( _document );

    allocator->free( _object, allocator->ud );
}
//////////////////////////////////////////////////////////////////////////
static void __js_node_destroy_from_pool( js_document_t * _document, js_node_t * _node )
{
    js_element_t * element = _node->element;

    __js_element_destroy( _document, element );
}
//////////////////////////////////////////////////////////////////////////
static void __js_node_destroy_allocator( js_document_t * _document, js_node_t * _node )
{
    js_allocator_t * allocator = __js_document_allocator( _document );

    js_element_t * element = _node->element;

    __js_element_destroy( _document, element );

    allocator->free( _node, allocator->ud );
}
//////////////////////////////////////////////////////////////////////////
static js_document_t * __js_document_create( js_allocator_t _allocator, js_flags_e _flags )
{
    js_document_t * document = (js_document_t *)_allocator.alloc( sizeof( js_document_t ), _allocator.ud );

    JS_ALLOCATOR_MEMORY_CHECK( document, JS_NULLPTR );

    document->object.base.type = js_type_object;

    document->object.size = 0;
    document->object.keys = JS_NULLPTR;
    document->object.values = JS_NULLPTR;

    document->allocator = _allocator;
    document->flags = _flags;

    if( _flags & js_flag_node_pool )
    {
        document->node_create = &__js_node_create_from_pool;
        document->node_destroy = &__js_node_destroy_from_pool;

        document->free_block = __js_block_create( &document->allocator, &document->free_node );
    }
    else
    {
        document->node_create = &__js_node_create_allocator;
        document->node_destroy = &__js_node_destroy_allocator;
    }

    if( _flags & js_flag_string_inplace )
    {
        document->string_create = &__js_string_create_inplace;
        document->string_destroy = &__js_string_destroy_inplace;
    }
    else
    {
        document->string_create = *__js_string_create_allocator;
        document->string_destroy = &__js_string_destroy_allocator;
    }

    return document;
}
//////////////////////////////////////////////////////////////////////////
static void __js_element_node_add( js_node_t ** _root, js_node_t * _node )
{
    _node->next = (*_root);
    (*_root) = _node;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_object_add( js_document_t * _document, js_object_t * _object, js_string_t * _key, js_element_t * _value )
{
    ++_object->size;

    js_node_t * key_node = _document->node_create( _document, (js_element_t *)_key );

    JS_ALLOCATOR_MEMORY_CHECK( key_node, JS_FAILURE );

    js_node_t * value_node = _document->node_create( _document, _value );

    JS_ALLOCATOR_MEMORY_CHECK( value_node, JS_FAILURE );

    __js_element_node_add( &_object->keys, key_node );
    __js_element_node_add( &_object->values, value_node );

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_array_add( js_document_t * _document, js_array_t * _array, js_element_t * _value )
{
    ++_array->size;

    js_node_t * value_node = _document->node_create( _document, _value );

    if( value_node == JS_NULLPTR )
    {
        return JS_FAILURE;
    }

    __js_element_node_add( &_array->values, value_node );

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static const char * __js_strpbrk( const char * _begin, const char * _end, const char * _str )
{
    const char * it = _begin;

    for( ; it != _end; ++it )
    {
        const char * it_str = _str;

        for( ; *it_str != '\0'; ++it_str )
        {
            if( *it == *it_str )
            {
                return it;
            }
        }
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
static js_bool_t __js_chrskip( char _ch, const char * _str )
{
    const char * it_str = _str;

    for( ; *it_str != '\0'; ++it_str )
    {
        if( _ch == *it_str )
        {
            return JS_TRUE;
        }
    }

    return JS_FALSE;
}
//////////////////////////////////////////////////////////////////////////
static const char * __js_strskip( const char * _begin, const char * _end, const char * _str )
{
    const char * it = _begin;

    for( ; it != _end; ++it )
    {
        if( __js_chrskip( *it, _str ) == JS_FALSE )
        {
            return it;
        }
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
static const char * __js_strstr( const char * _begin, const char * _end, const char * _str )
{
    if( *_str == '\0' )
    {
        return _begin;
    }

    const char * b = _str;
    const char * it = _begin;

    for( ; it != _end; ++it )
    {
        if( *it != *b )
        {
            continue;
        }

        const char * a = it;

        for( ;; )
        {
            if( *b == 0 )
            {
                return it;
            }

            if( *a++ != *b++ )
            {
                break;
            }
        }

        b = _str;
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
static const char * __js_strchr( const char * _begin, const char * _end, char _ch )
{
    const char * it = _begin;

    for( ; it != _end; ++it )
    {
        if( *it == _ch )
        {
            return it;
        }
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
static js_bool_t __js_strncmp( const char * _s1, const char * _s2, js_size_t _n )
{
    if( _n == 0 )
    {
        return JS_TRUE;
    }

    do
    {
        if( *_s1 != *_s2++ )
        {
            return JS_FALSE;
        }

        if( *_s1++ == '\0' )
        {
            break;
        }
    } while( --_n );

    return JS_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static js_bool_t __js_strzcmp( const char * _s1, js_size_t _z1, const char * _s2, js_size_t _z2 )
{
    if( _z1 != _z2 )
    {
        return JS_FALSE;
    }

    for( js_size_t index = 0; index != _z1; ++index )
    {
        if( _s1[index] != _s2[index] )
        {
            return JS_FALSE;
        }
    }

    return JS_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_parse_element( js_document_t * _document, const char ** _data, const char * _end, char _token, js_failed_fun_t _failed, void * _ud, js_element_t ** _element );
static js_result_t __js_parse_array( js_document_t * _document, const char ** _data, const char * _end, js_failed_fun_t _failed, void * _ud, js_array_t * _array );
static js_result_t __js_parse_object( js_document_t * _document, const char ** _data, const char * _end, js_failed_fun_t _failed, void * _ud, js_object_t * _object );
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_parse_element( js_document_t * _document, const char ** _data, const char * _end, char _token, js_failed_fun_t _failed, void * _ud, js_element_t ** _element )
{
    js_allocator_t * allocator = __js_document_allocator( _document );

    const char * data_begin = *_data;

    char brks[] = {',', '"', '{', '[', _token, '\0'};

    const char * data_soa = __js_strpbrk( data_begin, _end, brks );

    if( data_soa == JS_NULLPTR )
    {
        _failed( data_begin, _end, "parse element", _ud );

        return JS_FAILURE;
    }

    if( *data_soa == ',' || *data_soa == _token )
    {
        if( __js_strstr( data_begin, data_soa, "true" ) != JS_NULLPTR )
        {
            js_boolean_t * boolean = __js_boolean_create( allocator, JS_TRUE );

            *_element = (js_element_t *)boolean;

            *_data = data_begin + 4;

            return JS_SUCCESSFUL;
        }
        else if( __js_strstr( data_begin, data_soa, "false" ) != JS_NULLPTR )
        {
            js_boolean_t * boolean = __js_boolean_create( allocator, JS_FALSE );

            *_element = (js_element_t *)boolean;

            *_data = data_begin + 5;

            return JS_SUCCESSFUL;
        }
        else if( __js_strstr( data_begin, data_soa, "null" ) != JS_NULLPTR )
        {
            js_null_t * null = __js_null_create( allocator );

            *_element = (js_element_t *)null;

            *_data = data_begin + 4;

            return JS_SUCCESSFUL;
        }
        else
        {
            const char * data_real = __js_strpbrk( data_begin, data_soa, ".Ee" );

            if( data_real == JS_NULLPTR )
            {
                char * data_end;
                long long value = strtoll( data_begin, &data_end, 10 );

                if( data_begin == data_end )
                {
                    _failed( data_begin, _end, "parse element [integer]", _ud );

                    return JS_FAILURE;
                }

                js_integer_t * integer = __js_integer_create( allocator, value );

                JS_ALLOCATOR_MEMORY_CHECK( integer, JS_FAILURE );

                *_element = (js_element_t *)integer;

                *_data = data_end;

                return JS_SUCCESSFUL;
            }
            else
            {
                char * data_end;
                double value = strtod( data_begin, &data_end );

                if( data_begin == data_end )
                {
                    _failed( data_begin, _end, "parse element [real]", _ud );

                    return JS_FAILURE;
                }

                js_real_t * real = __js_real_create( allocator, value );

                JS_ALLOCATOR_MEMORY_CHECK( real, JS_FAILURE );

                *_element = (js_element_t *)real;

                *_data = data_end;

                return JS_SUCCESSFUL;
            }
        }
    }
    else if( *data_soa == '"' )
    {
        const char * data_eoa = __js_strchr( data_soa + 1, _end, '"' );

        if( data_eoa == JS_NULLPTR )
        {
            _failed( data_soa + 1, _end, "parse element [string]", _ud );

            return JS_FAILURE;
        }

        js_size_t data_size = data_eoa - data_soa;

        js_string_t * string = _document->string_create( allocator, data_soa + 1, data_size - 1 );

        JS_ALLOCATOR_MEMORY_CHECK( string, JS_FAILURE );

        *_element = (js_element_t *)string;

        *_data = data_eoa + 1;

        return JS_SUCCESSFUL;
    }
    else if( *data_soa == '{' )
    {
        const char * data_iterator = data_soa;

        js_object_t * object = __js_object_create( allocator );

        JS_ALLOCATOR_MEMORY_CHECK( object, JS_FAILURE );

        if( __js_parse_object( _document, &data_iterator, _end, _failed, _ud, object ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        *_element = (js_element_t *)object;

        *_data = data_iterator;

        return JS_SUCCESSFUL;
    }
    else if( *data_soa == '[' )
    {
        const char * data_iterator = data_soa;

        js_array_t * array = __js_array_create( allocator );

        JS_ALLOCATOR_MEMORY_CHECK( array, JS_FAILURE );

        if( __js_parse_array( _document, &data_iterator, _end, _failed, _ud, array ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        *_element = (js_element_t *)array;

        *_data = data_iterator;

        return JS_SUCCESSFUL;
    }

    _failed( data_begin, _end, "parse element", _ud );

    return JS_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_parse_array( js_document_t * _document, const char ** _data, const char * _end, js_failed_fun_t _failed, void * _ud, js_array_t * _array )
{
    const char * data_begin = *_data;

    const char * data_iterator = data_begin + 1;

    const char * value_empty = __js_strskip( data_iterator, _end, " \t\n\r" );

    if( value_empty != JS_NULLPTR && *value_empty == ']' )
    {
        *_data = value_empty + 1;

        return JS_SUCCESSFUL;
    }

    for( ;; )
    {
        js_element_t * value;
        if( __js_parse_element( _document, &data_iterator, _end, ']', _failed, _ud, &value ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        if( __js_array_add( _document, _array, value ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        const char * value_end = __js_strpbrk( data_iterator, _end, ",]" );

        if( *value_end == ']' )
        {
            *_data = value_end + 1;

            break;
        }

        data_iterator = value_end + 1;
    }

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_parse_object( js_document_t * _document, const char ** _data, const char * _end, js_failed_fun_t _failed, void * _ud, js_object_t * _object )
{
    js_allocator_t * allocator = __js_document_allocator( _document );

    const char * data_begin = *_data;
    const char * data_iterator = data_begin + 1;

    const char * value_empty = __js_strskip( data_iterator, _end, " \t\n\r" );

    if( value_empty != JS_NULLPTR && *value_empty == '}' )
    {
        *_data = value_empty + 1;

        return JS_SUCCESSFUL;
    }

    for( ;; )
    {
        const char * key_begin = __js_strchr( data_iterator, _end, '"' );

        if( key_begin == JS_NULLPTR )
        {
            _failed( data_iterator, _end, "parse object [key begin]", _ud );

            return JS_FAILURE;
        }

        const char * key_end = __js_strchr( key_begin + 1, _end, '"' );

        if( key_end == JS_NULLPTR )
        {
            _failed( key_begin + 1, _end, "parse object [key end]", _ud );

            return JS_FAILURE;
        }

        js_size_t key_size = key_end - key_begin;

        js_string_t * key = _document->string_create( allocator, key_begin + 1, key_size - 1 );

        JS_ALLOCATOR_MEMORY_CHECK( key, JS_FAILURE );

        const char * value_begin = __js_strchr( key_end + 1, _end, ':' );

        if( value_begin == JS_NULLPTR )
        {
            _failed( key_end + 1, _end, "parse object [value separator]", _ud );

            return JS_FAILURE;
        }

        const char * value_iterator = value_begin + 1;

        js_element_t * value;
        if( __js_parse_element( _document, &value_iterator, _end, '}', _failed, _ud, &value ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        if( __js_object_add( _document, _object, key, value ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        const char * value_end = __js_strpbrk( value_iterator, _end, ",}" );

        if( *value_end == '}' )
        {
            *_data = value_end + 1;

            break;
        }

        data_iterator = value_end + 1;
    }

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void * __js_buffer_alloc( size_t _size, void * _ud )
{
    js_buffer_t * buffer = (js_buffer_t *)_ud;

    if( buffer->memory + _size > buffer->end )
    {
        return JS_NULLPTR;
    }

    void * alloc_memory = buffer->memory;

    buffer->memory += _size;

    return alloc_memory;
}
//////////////////////////////////////////////////////////////////////////
static void __js_buffer_free( void * _ptr, void * _ud )
{
    JS_UNUSED( _ptr );
    JS_UNUSED( _ud );
}
//////////////////////////////////////////////////////////////////////////
void js_make_buffer( void * _memory, js_size_t _capacity, js_buffer_t * const _buffer )
{
    _buffer->memory = (uint8_t *)_memory;
    _buffer->end = (uint8_t *)_memory + _capacity;
}
//////////////////////////////////////////////////////////////////////////
void js_make_allocator_buffer( js_buffer_t * _buffer, js_allocator_t * const _allocator )
{
    _allocator->alloc = &__js_buffer_alloc;
    _allocator->free = &__js_buffer_free;
    _allocator->ud = _buffer;
}
//////////////////////////////////////////////////////////////////////////
void js_make_allocator_default( js_alloc_fun_t _alloc, js_free_fun_t _free, void * ud, js_allocator_t * const _allocator )
{
    _allocator->alloc = _alloc;
    _allocator->free = _free;
    _allocator->ud = ud;
}
//////////////////////////////////////////////////////////////////////////
js_result_t js_parse( js_allocator_t _allocator, js_flags_e _flags, const char * _data, js_size_t _size, js_failed_fun_t _failed, void * _ud, js_element_t ** _element )
{
    const char * data_begin = _data;
    const char * data_end = _data + _size;

    const char * data_root = __js_strchr( data_begin, data_end, '{' );

    if( data_root == JS_NULLPTR )
    {
        if( _failed != JS_NULLPTR )
        {
            _failed( data_begin, data_end, "parse root [begin]", _ud );
        }

        return JS_FAILURE;
    }

    const char * data_iterator = data_root;

    js_document_t * document = __js_document_create( _allocator, _flags );

    JS_ALLOCATOR_MEMORY_CHECK( document, JS_FAILURE );

    if( __js_parse_object( document, &data_iterator, data_end, _failed, _ud, (js_object_t *)document ) == JS_FAILURE )
    {
        return JS_FAILURE;
    }

    *_element = (js_element_t *)document;

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_clone_element( js_document_t * _document, const js_element_t * _element, js_element_t ** _clone );
static js_result_t __js_clone_array( js_document_t * _document, js_array_t * _clone, const js_array_t * _base );
static js_result_t __js_clone_object( js_document_t * _document, js_object_t * _clone, const js_object_t * _base );
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_clone_element( js_document_t * _document, const js_element_t * _element, js_element_t ** _clone )
{
    js_allocator_t * allocator = __js_document_allocator( _document );

    js_type_e type = js_type( _element );

    switch( type )
    {
    case js_type_null:
        {
            js_null_t * null = __js_null_create( allocator );

            JS_ALLOCATOR_MEMORY_CHECK( null, JS_FAILURE );

            *_clone = (js_element_t *)null;
        }break;
    case js_type_boolean:
        {
            js_boolean_t * boolean = (js_boolean_t *)_element;

            js_boolean_t * boolean_clone = __js_boolean_create( allocator, boolean->value );

            JS_ALLOCATOR_MEMORY_CHECK( boolean_clone, JS_FAILURE );

            *_clone = (js_element_t *)boolean_clone;
        }break;
    case js_type_integer:
        {
            js_integer_t * integer = (js_integer_t *)_element;

            js_integer_t * integer_clone = __js_integer_create( allocator, integer->value );

            JS_ALLOCATOR_MEMORY_CHECK( integer_clone, JS_FAILURE );

            *_clone = (js_element_t *)integer_clone;
        }break;
    case js_type_real:
        {
            js_real_t * real = (js_real_t *)_element;

            js_real_t * real_clone = __js_real_create( allocator, real->value );

            JS_ALLOCATOR_MEMORY_CHECK( real_clone, JS_FAILURE );

            *_clone = (js_element_t *)real_clone;
        }break;
    case js_type_string:
        {
            js_string_t * string = (js_string_t *)_element;

            js_string_t * string_clone = _document->string_create( allocator, string->value, string->size );

            JS_ALLOCATOR_MEMORY_CHECK( string_clone, JS_FAILURE );

            *_clone = (js_element_t *)string_clone;
        }break;
    case js_type_array:
        {
            js_array_t * array = (js_array_t *)_element;

            js_array_t * array_clone = __js_array_create( allocator );

            JS_ALLOCATOR_MEMORY_CHECK( array_clone, JS_FAILURE );

            if( __js_clone_array( _document, array_clone, array ) == JS_FAILURE )
            {
                return JS_FAILURE;
            }

            *_clone = (js_element_t *)array_clone;
        }break;
    case js_type_object:
        {
            js_object_t * object = (js_object_t *)_element;

            js_object_t * object_clone = __js_object_create( allocator );

            JS_ALLOCATOR_MEMORY_CHECK( object_clone, JS_FAILURE );

            if( __js_clone_object( _document, object_clone, object ) == JS_FAILURE )
            {
                return JS_FAILURE;
            }

            *_clone = (js_element_t *)object_clone;
        }break;
    }

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_clone_array( js_document_t * _document, js_array_t * _clone, const js_array_t * _base )
{
    js_node_t * it_value = _base->values;

    for( ; it_value != JS_NULLPTR; it_value = it_value->next )
    {
        const js_element_t * value = it_value->element;

        js_element_t * value_clone;
        if( __js_clone_element( _document, value, &value_clone ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        if( __js_array_add( _document, _clone, value_clone ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }
    }

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_clone_object( js_document_t * _document, js_object_t * _clone, const js_object_t * _base )
{
    js_allocator_t * allocator = __js_document_allocator( _document );

    js_node_t * it_key = _base->keys;
    js_node_t * it_value = _base->values;

    for( ; it_key != JS_NULLPTR; it_key = it_key->next, it_value = it_value->next )
    {
        const js_string_t * key = (const js_string_t *)it_key->element;
        const js_element_t * value = it_value->element;

        js_string_t * key_clone = _document->string_create( allocator, key->value, key->size );

        JS_ALLOCATOR_MEMORY_CHECK( key_clone, JS_FAILURE );

        js_element_t * value_clone;
        if( __js_clone_element( _document, value, &value_clone ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        if( __js_object_add( _document, _clone, key_clone, value_clone ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }
    }

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
js_result_t js_clone( js_allocator_t _allocator, js_flags_e _flags, const js_element_t * _base, js_element_t ** _total )
{
    js_document_t * document = __js_document_create( _allocator, _flags );

    JS_ALLOCATOR_MEMORY_CHECK( document, JS_FAILURE );

    const js_object_t * base = (const js_object_t *)_base;

    if( __js_clone_object( document, (js_object_t *)document, base ) == JS_FAILURE )
    {
        return JS_FAILURE;
    }

    *_total = (js_element_t *)document;

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_patch_object( js_document_t * _document, js_object_t * _object, const js_object_t * _patch )
{
    js_allocator_t * allocator = __js_document_allocator( _document );

    js_node_t * it_object_key = _object->keys;
    js_node_t * it_object_value = _object->values;

    for( ; it_object_key != JS_NULLPTR; it_object_key = it_object_key->next, it_object_value = it_object_value->next )
    {
        const js_string_t * object_key = (const js_string_t *)it_object_key->element;
        js_element_t * object_value = it_object_value->element;

        const js_node_t * it_patch_key = _patch->keys;
        const js_node_t * it_patch_value = _patch->values;

        for( ; it_patch_key != JS_NULLPTR; it_patch_key = it_patch_key->next, it_patch_value = it_patch_value->next )
        {
            const js_string_t * patch_key = (const js_string_t *)it_patch_key->element;
            const js_element_t * patch_value = it_patch_value->element;

            if( __js_strzcmp( object_key->value, object_key->size, patch_key->value, patch_key->size ) == JS_FALSE )
            {
                continue;
            }
            
            js_type_e object_value_type = js_type( object_value );
            js_type_e patch_value_type = js_type( patch_value );

            if( object_value_type == js_type_object && patch_value_type == js_type_object )
            {
                if( __js_patch_object( _document, (js_object_t *)object_value, (const js_object_t *)patch_value ) == JS_FAILURE )
                {
                    return JS_FAILURE;
                }
            }
            else if( patch_value_type == js_type_null )
            {
                js_node_t * prev_object_key = _object->keys;
                js_node_t * prev_object_value = _object->values;

                if( prev_object_key == it_object_key )
                {
                    js_node_t * next_keys = it_object_key->next;
                    js_node_t * next_values = it_object_value->next;

                    _document->node_destroy( _document, it_object_key );
                    _document->node_destroy( _document, it_object_value );

                    _object->keys = next_keys;
                    _object->values = next_values;

                    it_object_key = next_keys;
                    it_object_value = next_values;
                }
                else
                {
                    for( ; prev_object_key->next != it_object_key; prev_object_key = prev_object_key->next, prev_object_value = prev_object_value->next );

                    js_node_t * next_keys = it_object_key->next;
                    js_node_t * next_values = it_object_value->next;

                    _document->node_destroy( _document, it_object_key );
                    _document->node_destroy( _document, it_object_value );

                    prev_object_key->next = next_keys;
                    prev_object_value->next = next_values;
                }
            }
            else
            {
                js_element_t * patch_value_clone;
                if( __js_clone_element( _document, patch_value, &patch_value_clone ) == JS_FAILURE )
                {
                    return JS_FAILURE;
                }

                __js_element_destroy( _document, object_value );

                it_object_value->element = patch_value_clone;
            }
        }
    }

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
js_result_t js_patch( js_allocator_t _allocator, js_flags_e _flags, const js_element_t * _base, const js_element_t * _patch, js_element_t ** _total )
{
    js_document_t * document = __js_document_create( _allocator, _flags );

    JS_ALLOCATOR_MEMORY_CHECK( document, JS_FAILURE );

    const js_object_t * base = (const js_object_t *)_base;
    const js_object_t * patch = (const js_object_t *)_patch;

    if( __js_clone_object( document, (js_object_t *)document, base ) == JS_FAILURE )
    {
        return JS_FAILURE;
    }

    if( __js_patch_object( document, (js_object_t *)document, patch ) == JS_FAILURE )
    {
        return JS_FAILURE;
    }

    *_total = (js_element_t *)document;

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void js_free( js_element_t * _element )
{
    js_document_t * document = (js_document_t *)_element;

    js_node_t * it_key = document->object.keys;
    js_node_t * it_value = document->object.values;

    for( ; it_key != JS_NULLPTR; )
    {
        js_node_t * free_key = it_key;
        js_node_t * free_value = it_value;

        it_key = it_key->next;
        it_value = it_value->next;

        document->node_destroy( document, free_key );
        document->node_destroy( document, free_value );
    }

    js_allocator_t * allocator = __js_document_allocator( document );

    js_flags_e flags = document->flags;

    if( flags & js_flag_node_pool )
    {
        for( js_block_t * block = document->free_block; block != JS_NULLPTR; )
        {
            js_block_t * free_block = block;
            block = block->prev;

            allocator->free( free_block, allocator->ud );
        }
    }

    allocator->free( document, allocator->ud );
}
//////////////////////////////////////////////////////////////////////////
js_type_e js_type( const js_element_t * _element )
{
    js_type_e type = _element->type;

    return type;
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_is_null( const js_element_t * _object )
{
    return _object->type == js_type_null;
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_is_boolean( const js_element_t * _object )
{
    return _object->type == js_type_boolean;
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_is_integer( const js_element_t * _object )
{
    return _object->type == js_type_integer;
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_is_real( const js_element_t * _object )
{
    return _object->type == js_type_real;
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_is_string( const js_element_t * _object )
{
    return _object->type == js_type_string;
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_is_array( const js_element_t * _object )
{
    return _object->type == js_type_array;
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_is_object( const js_element_t * _object )
{
    return _object->type == js_type_object;
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_get_boolean( const js_element_t * _element )
{
    js_boolean_t * el = (js_boolean_t *)_element;

    js_bool_t value = el->value;

    return value;
}
//////////////////////////////////////////////////////////////////////////
js_integer_value_t js_get_integer( const js_element_t * _element )
{
    js_integer_t * el = (js_integer_t *)_element;

    js_integer_value_t value = el->value;

    return value;
}
//////////////////////////////////////////////////////////////////////////
js_real_value_t js_get_real( const js_element_t * _element )
{
    js_real_t * el = (js_real_t *)_element;

    js_real_value_t value = el->value;

    return value;
}
//////////////////////////////////////////////////////////////////////////
void js_get_string( const js_element_t * _element, const char ** _value, js_size_t * const _size )
{
    js_string_t * el = (js_string_t *)_element;

    *_value = el->value;
    *_size = el->size;
}
//////////////////////////////////////////////////////////////////////////
js_size_t js_array_size( const js_element_t * _element )
{
    const js_array_t * array = (const js_array_t *)_element;

    js_size_t size = array->size;

    return size;
}
//////////////////////////////////////////////////////////////////////////
const js_element_t * js_array_get( const js_element_t * _element, js_size_t _index )
{
    const js_array_t * array = (const js_array_t *)_element;

    const js_node_t * it = array->values;

    for( js_size_t index = 0; index != _index; ++index )
    {
        it = it->next;
    }

    const js_element_t * value = it->element;

    return value;
}
//////////////////////////////////////////////////////////////////////////
js_size_t js_object_size( const js_element_t * _object )
{
    const js_object_t * object = (const js_object_t *)_object;

    js_size_t size = object->size;

    return size;
}
//////////////////////////////////////////////////////////////////////////
const js_element_t * js_object_get( const js_element_t * _element, const char * _key )
{
    const js_object_t * object = (const js_object_t *)_element;

    const js_node_t * it_key = object->keys;
    const js_node_t * it_value = object->values;

    for( ; it_key != JS_NULLPTR; it_key = it_key->next, it_value = it_value->next )
    {
        const js_string_t * key = (const js_string_t *)it_key->element;

        const char * key_value = key->value;
        js_size_t key_size = key->size;

        if( __js_strncmp( _key, key_value, key_size ) == JS_FALSE )
        {
            continue;
        }

        const js_element_t * value = it_value->element;

        return value;
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const js_element_t * js_object_getn( const js_element_t * _object, const char * _key, js_size_t _size )
{
    const js_object_t * object = (const js_object_t *)_object;

    const js_node_t * it_key = object->keys;
    const js_node_t * it_value = object->values;

    for( ; it_key != JS_NULLPTR; it_key = it_key->next, it_value = it_value->next )
    {
        const js_string_t * key = (const js_string_t *)it_key->element;

        const char * key_value = key->value;
        js_size_t key_size = key->size;

        if( __js_strzcmp( _key, _size, key_value, key_size ) == JS_FALSE )
        {
            continue;
        }

        const js_element_t * value = it_value->element;

        return value;
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
void js_object_foreach( const js_element_t * _element, js_object_foreach_t _foreach, void * _ud )
{
    const js_object_t * object = (const js_object_t *)_element;

    const js_node_t * it_key = object->keys;
    const js_node_t * it_value = object->values;

    size_t index = 0;

    for( ; it_key != JS_NULLPTR; it_key = it_key->next, it_value = it_value->next )
    {
        const js_string_t * key = (const js_string_t * )it_key->element;
        const js_element_t * value = it_value->element;

        const char * key_value = key->value;
        js_size_t key_size = key->size;

        _foreach( index, key_value, key_size, value, _ud );

        ++index;
    }
}
//////////////////////////////////////////////////////////////////////////
void js_array_foreach( const js_element_t * _element, js_array_foreach_t _foreach, void * _ud )
{
    const js_array_t * array = (const js_array_t *)_element;

    const js_node_t * it_value = array->values;

    size_t index = 0;

    for( ; it_value != JS_NULLPTR; it_value = it_value->next )
    {
        const js_element_t * value = it_value->element;

        _foreach( index, value, _ud );

        ++index;
    }
}
//////////////////////////////////////////////////////////////////////////