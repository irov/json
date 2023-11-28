#include "json.h"

#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
typedef struct js_element_t
{
    js_type_e type;
    struct js_element_t * next;
    struct js_element_t * prev;
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
    int64_t value;
} js_integer_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_real_t
{
    js_element_t base;
    double value;
} js_real_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_string_t
{
    js_element_t base;
    const char * value;
    js_size_t size;
} js_string_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_object_t
{
    js_element_t base;
    js_size_t size;
    js_string_t * keys;
    js_element_t * values;
} js_object_t;
//////////////////////////////////////////////////////////////////////////
typedef struct js_array_t
{
    js_element_t base;
    js_size_t size;
    js_element_t * values;
} js_array_t;
//////////////////////////////////////////////////////////////////////////
static js_null_t * __js_null_create( js_allocator_t * _allocator )
{
    js_null_t * null = (js_null_t *)_allocator->alloc( sizeof( js_null_t ), _allocator->ud );

    null->base.type = js_type_null;
    null->base.next = JS_NULLPTR;
    null->base.prev = JS_NULLPTR;

    return null;
}
//////////////////////////////////////////////////////////////////////////
static js_boolean_t * __js_boolean_create( js_allocator_t * _allocator, js_bool_t _value )
{
    js_boolean_t * boolean = (js_boolean_t *)_allocator->alloc( sizeof( js_boolean_t ), _allocator->ud );

    boolean->base.type = js_type_boolean;
    boolean->base.next = JS_NULLPTR;
    boolean->base.prev = JS_NULLPTR;

    boolean->value = _value;

    return boolean;
}
//////////////////////////////////////////////////////////////////////////
static js_integer_t * __js_integer_create( js_allocator_t * _allocator, int64_t _value )
{
    js_integer_t * integer = (js_integer_t *)_allocator->alloc( sizeof( js_integer_t ), _allocator->ud );

    integer->base.type = js_type_integer;
    integer->base.next = JS_NULLPTR;
    integer->base.prev = JS_NULLPTR;

    integer->value = _value;

    return integer;
}
//////////////////////////////////////////////////////////////////////////
static js_real_t * __js_real_create( js_allocator_t * _allocator, double _value )
{
    js_real_t * real = (js_real_t *)_allocator->alloc( sizeof( js_real_t ), _allocator->ud );

    real->base.type = js_type_real;
    real->base.next = JS_NULLPTR;
    real->base.prev = JS_NULLPTR;

    real->value = _value;

    return real;
}
//////////////////////////////////////////////////////////////////////////
static js_string_t * __js_string_create( js_allocator_t * _allocator, const char * _value, js_size_t _size )
{
    js_string_t * string = (js_string_t *)_allocator->alloc( sizeof( js_string_t ), _allocator->ud );

    string->base.type = js_type_string;
    string->base.next = JS_NULLPTR;
    string->base.prev = JS_NULLPTR;

    if( _allocator->flags & js_flag_string_inplace )
    {
        string->value = _value;
        string->size = _size;
    }
    else
    {
        char * value = (char *)_allocator->alloc( _size + 1, _allocator->ud );
        for( js_size_t index = 0; index != _size; ++index )
        {
            value[index] = _value[index];
        }

        value[_size] = '\0';

        string->value = value;
        string->size = _size;
    }

    return string;
}
//////////////////////////////////////////////////////////////////////////
static js_object_t * __js_object_create( js_allocator_t * _allocator )
{
    js_object_t * object = (js_object_t *)_allocator->alloc( sizeof( js_object_t ), _allocator->ud );

    object->base.type = js_type_object;
    object->base.next = JS_NULLPTR;
    object->base.prev = JS_NULLPTR;

    object->size = 0;
    object->keys = JS_NULLPTR;
    object->values = JS_NULLPTR;

    return object;
}
//////////////////////////////////////////////////////////////////////////
static js_array_t * __js_array_create( js_allocator_t * _allocator )
{
    js_array_t * array = (js_array_t *)_allocator->alloc( sizeof( js_array_t ), _allocator->ud );

    array->base.type = js_type_array;
    array->base.next = JS_NULLPTR;
    array->base.prev = JS_NULLPTR;

    array->size = 0;
    array->values = JS_NULLPTR;

    return array;
}
//////////////////////////////////////////////////////////////////////////
static void __js_object_add( js_object_t * _object, js_string_t * _key, js_element_t * _value )
{
    ++_object->size;

    if( _object->keys == JS_NULLPTR )
    {
        _object->keys = _key;
        _object->values = _value;

        _key->base.next = JS_NULLPTR;
        _key->base.prev = (js_element_t *)_key;

        _value->next = JS_NULLPTR;
        _value->prev = _value;

        return;
    }
    else
    {
        _key->base.prev = _object->keys->base.prev;
        _key->base.next = JS_NULLPTR;

        _object->keys->base.prev->next = (js_element_t *)_key;
        _object->keys->base.prev = (js_element_t *)_key;

        _value->prev = _object->values->prev;
        _value->next = JS_NULLPTR;

        _object->values->prev->next = _value;
        _object->values->prev = _value;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __js_array_add( js_array_t * _array, js_element_t * _value )
{
    ++_array->size;

    if( _array->values == JS_NULLPTR )
    {
        _array->values = _value;

        _value->next = JS_NULLPTR;
        _value->prev = _value;

        return;
    }
    else
    {
        _value->prev = _array->values->prev;
        _value->next = JS_NULLPTR;

        _array->values->prev->next = _value;
        _array->values->prev = _value;
    }
}
//////////////////////////////////////////////////////////////////////////
static const char * js_strpbrk( const char * _begin, const char * _end, const char * _str )
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
static js_bool_t js_chrskip( char _ch, const char * _str )
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
static const char * js_strskip( const char * _begin, const char * _end, const char * _str )
{
    const char * it = _begin;

    for( ; it != _end; ++it )
    {
        const char * it_str = _str;

        if( js_chrskip( *it, _str ) == JS_FALSE )
        {
            return it;
        }
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
static const char * js_strstr( const char * _begin, const char * _end, const char * _str )
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
static const char * js_strchr( const char * _begin, const char * _end, char _ch )
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
static js_bool_t js_strncmp( const char * _s1, const char * _s2, js_size_t _n )
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
static js_bool_t js_strzcmp( const char * _s1, js_size_t _z1, const char * _s2, js_size_t _z2 )
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
static js_result_t __js_parse_element( js_allocator_t * _allocator, const char ** _data, const char * _end, char _token, js_element_t ** _element );
static js_result_t __js_parse_array( js_allocator_t * _allocator, const char ** _data, const char * _end, js_array_t ** _array );
static js_result_t __js_parse_object( js_allocator_t * _allocator, const char ** _data, const char * _end, js_object_t ** _object );
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_parse_element( js_allocator_t * _allocator, const char ** _data, const char * _end, char _token, js_element_t ** _element )
{
    const char * data_begin = *_data;

    char brks[] = {',', '"', '{', '[', _token};

    const char * data_soa = js_strpbrk( data_begin, _end, brks );

    if( data_soa == JS_NULLPTR )
    {
        return JS_FAILURE;
    }

    if( *data_soa == ',' || *data_soa == _token )
    {
        if( js_strstr( data_begin, _end, "true" ) != JS_NULLPTR )
        {
            js_boolean_t * boolean = __js_boolean_create( _allocator, JS_TRUE );

            *_element = (js_element_t *)boolean;

            return JS_SUCCESSFUL;
        }
        else if( js_strstr( data_begin, _end, "false" ) != JS_NULLPTR )
        {
            js_boolean_t * boolean = __js_boolean_create( _allocator, JS_FALSE );

            *_element = (js_element_t *)boolean;

            return JS_SUCCESSFUL;
        }
        else if( js_strstr( data_begin, _end, "null" ) != JS_NULLPTR )
        {
            js_null_t * null = __js_null_create( _allocator );

            *_element = (js_element_t *)null;

            return JS_SUCCESSFUL;
        }
        else
        {
            const char * data_real = js_strpbrk( data_begin, data_soa, ".Ee" );

            if( data_real == JS_NULLPTR )
            {
                char * data_end;
                int64_t value = strtoll( data_begin, &data_end, 10 );

                if( data_begin == data_end )
                {
                    return JS_FAILURE;
                }

                js_integer_t * integer = __js_integer_create( _allocator, value );

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
                    return JS_FAILURE;
                }

                js_real_t * real = __js_real_create( _allocator, value );

                *_element = (js_element_t *)real;

                *_data = data_end;

                return JS_SUCCESSFUL;
            }
        }
    }
    else if( *data_soa == '"' )
    {
        const char * data_eoa = js_strchr( data_soa + 1, _end, '"' );
        if( data_eoa == JS_NULLPTR )
        {
            return JS_FAILURE;
        }

        js_size_t data_size = data_eoa - data_soa;

        js_string_t * string = __js_string_create( _allocator, data_soa + 1, data_size - 1 );
        if( string == JS_NULLPTR )
        {
            return JS_FAILURE;
        }

        *_element = (js_element_t *)string;

        *_data = data_eoa + 1;

        return JS_SUCCESSFUL;
    }
    else if( *data_soa == '{' )
    {
        js_object_t * object;
        if( __js_parse_object( _allocator, _data, _end, &object ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        *_element = (js_element_t *)object;

        return JS_SUCCESSFUL;
    }
    else if( *data_soa == '[' )
    {
        js_array_t * array;
        if( __js_parse_array( _allocator, _data, _end, &array ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        *_element = (js_element_t *)array;

        return JS_SUCCESSFUL;
    }

    return JS_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_parse_array( js_allocator_t * _allocator, const char ** _data, const char * _end, js_array_t ** _array )
{
    js_array_t * array = __js_array_create( _allocator );

    const char * data_begin = *_data;

    const char * data_iterator = data_begin + 1;

    const char * value_empty = js_strskip( data_iterator, _end, " \t\n\r" );

    if( value_empty != JS_NULLPTR && *value_empty == ']' )
    {
        *_data = value_empty + 1;

        *_array = array;

        return JS_SUCCESSFUL;
    }

    for( ;; )
    {
        js_element_t * value;
        if( __js_parse_element( _allocator, &data_iterator, _end, ']', &value ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        __js_array_add( array, value );

        const char * value_end = js_strpbrk( data_iterator, _end, ",]" );

        if( *value_end == ']' )
        {
            *_data = value_end + 1;

            break;
        }

        data_iterator = value_end + 1;
    }

    *_array = array;

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_parse_object( js_allocator_t * _allocator, const char ** _data, const char * _end, js_object_t ** _object )
{
    js_object_t * object = __js_object_create( _allocator );

    const char * data_begin = *_data;
    const char * data_iterator = data_begin + 1;

    const char * value_empty = js_strskip( data_iterator, _end, " \t\n\r" );

    if( value_empty != JS_NULLPTR && *value_empty == '}' )
    {
        *_data = value_empty + 1;

        *_object = object;

        return JS_SUCCESSFUL;
    }

    for( ;; )
    {
        const char * key_begin = js_strchr( data_iterator, _end, '"' );
        if( key_begin == JS_NULLPTR )
        {
            return JS_FAILURE;
        }

        const char * key_end = js_strchr( key_begin + 1, _end, '"' );
        if( key_end == JS_NULLPTR )
        {
            return JS_FAILURE;
        }

        js_size_t key_size = key_end - key_begin;

        js_string_t * key = __js_string_create( _allocator, key_begin + 1, key_size - 1 );
        if( key == JS_NULLPTR )
        {
            return JS_FAILURE;
        }

        const char * value_begin = js_strchr( key_end + 1, _end, ':' );
        if( value_begin == JS_NULLPTR )
        {
            return JS_FAILURE;
        }

        const char * value_iterator = value_begin + 1;

        js_element_t * value;
        if( __js_parse_element( _allocator, &value_iterator, _end, '}', &value ) == JS_FAILURE )
        {
            return JS_FAILURE;
        }

        __js_object_add( object, key, value );

        const char * value_end = js_strpbrk( value_iterator, _end, ",}" );

        if( *value_end == '}' )
        {
            *_data = value_end + 1;

            break;
        }

        data_iterator = value_end + 1;
    }

    *_object = object;

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
js_result_t js_parse( js_allocator_t * _allocator, const void * _data, js_size_t _size, js_element_t ** _element )
{
    const char * data = (const char *)_data;
    const char * end = data + _size;

    const char * data_begin = js_strchr( data, end, '{' );

    if( data_begin == JS_NULLPTR )
    {
        return JS_FAILURE;
    }

    js_object_t * object;
    if( __js_parse_object( _allocator, &data_begin, end, &object ) == JS_FAILURE )
    {
        return JS_FAILURE;
    }

    *_element = (js_element_t *)object;

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __js_free_element( js_allocator_t * _allocator, js_element_t * _element );
static void __js_free_array( js_allocator_t * _allocator, js_array_t * _array );
static void __js_free_object( js_allocator_t * _allocator, js_object_t * _object );
//////////////////////////////////////////////////////////////////////////
static void __js_free_string( js_allocator_t * _allocator, js_string_t * _string )
{
    if( _allocator->flags & js_flag_string_inplace )
    {
        _allocator->free( (void *)_string->value, _allocator->ud );
    }

    _allocator->free( _string, _allocator->ud );
}
//////////////////////////////////////////////////////////////////////////
static void __js_free_element( js_allocator_t * _allocator, js_element_t * _element )
{
    js_type_e type = js_type( _element );

    if( type & 0x8 )
    {
        _allocator->free( _element, _allocator->ud );
    } 
    else if( type == js_type_string )
    {
        js_string_t * string = (js_string_t *)_element;

        __js_free_string( _allocator, string );
    } 
    else if( type == js_type_array )
    {
        js_array_t * array = (js_array_t *)_element;

        __js_free_array( _allocator, array );
    }
    else if( type == js_type_object )
    {
        js_object_t * object = (js_object_t *)_element;

        __js_free_object( _allocator, object );
    }
}
//////////////////////////////////////////////////////////////////////////
static void __js_free_array( js_allocator_t * _allocator, js_array_t * _array )
{
    js_element_t * it_value = _array->values;

    for( ; it_value != JS_NULLPTR; )
    {
        js_element_t * free_value = it_value;

        it_value = it_value->next;
        
        __js_free_element( _allocator, free_value );
    }
}
//////////////////////////////////////////////////////////////////////////
static void __js_free_object( js_allocator_t * _allocator, js_object_t * _object )
{
    js_string_t * it_key = _object->keys;
    js_element_t * it_value = _object->values;

    for( ; it_key != JS_NULLPTR; )
    {
        js_string_t * free_key = it_key;
        js_element_t * free_value = it_value;

        it_key = (js_string_t *)it_key->base.next;
        it_value = it_value->next;

        __js_free_string( _allocator, free_key );
        __js_free_element( _allocator, free_value );
    }

    _allocator->free( _object, _allocator->ud );
}
//////////////////////////////////////////////////////////////////////////
void js_free( js_allocator_t * _allocator, js_element_t * _element )
{
    js_object_t * object = (js_object_t *)_element;

    __js_free_object( _allocator, object );
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
int64_t js_get_integer( const js_element_t * _element )
{
    js_integer_t * el = (js_integer_t *)_element;

    int64_t value = el->value;

    return value;
}
//////////////////////////////////////////////////////////////////////////
double js_get_real( const js_element_t * _element )
{
    js_real_t * el = (js_real_t *)_element;

    double value = el->value;

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

    const js_element_t * it = array->values;

    for( js_size_t index = 0; index != _index; ++index )
    {
        it = it->next;
    }

    return it;
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

    const js_string_t * it_key = object->keys;
    const js_element_t * it_value = object->values;

    for( ; it_key != JS_NULLPTR; it_key = (const js_string_t *)it_key->base.next, it_value = it_value->next )
    {
        const char * key_value = it_key->value;
        js_size_t key_size = it_key->size;

        if( js_strncmp( _key, key_value, key_size ) == JS_FALSE )
        {
            continue;
        }

        return it_value;
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const js_element_t * js_object_getn( const js_element_t * _object, const char * _key, js_size_t _size )
{
    const js_object_t * object = (const js_object_t *)_object;

    const js_string_t * it_key = object->keys;
    const js_element_t * it_value = object->values;

    for( ; it_key != JS_NULLPTR; it_key = (const js_string_t *)it_key->base.next, it_value = it_value->next )
    {
        const char * key_value = it_key->value;
        js_size_t key_size = it_key->size;

        if( js_strzcmp( _key, _size, key_value, key_size ) == JS_FALSE )
        {
            continue;
        }

        return it_value;
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
void js_object_foreach( const js_element_t * _element, js_object_foreach_t _foreach, void * _ud )
{
    const js_object_t * object = (const js_object_t *)_element;

    const js_string_t * it_key = object->keys;
    const js_element_t * it_value = object->values;

    size_t index = 0;

    for( ; it_key != JS_NULLPTR; it_key = (js_string_t *)it_key->base.next, it_value = it_value->next )
    {
        _foreach( index, it_key->value, it_key->size, it_value, _ud );

        ++index;
    }
}
//////////////////////////////////////////////////////////////////////////
void js_array_foreach( const js_element_t * _element, js_array_foreach_t _foreach, void * _ud )
{
    const js_array_t * array = (const js_array_t *)_element;

    const js_element_t * it_value = array->values;

    size_t index = 0;

    for( ; it_value != JS_NULLPTR; it_value = it_value->next )
    {
        _foreach( index, it_value, _ud );

        ++index;
    }
}
//////////////////////////////////////////////////////////////////////////