#include "json/json_dump.h"

#include "json/json.h"

#include "json_string.h"

#define JS_DUMP( _ctx, _size ) ((char *)(*_ctx->buffer)(_size, _ctx->ud))

//////////////////////////////////////////////////////////////////////////
static void __js_dump_char( js_dump_ctx_t * _ctx, char _value )
{
    char * dst = JS_DUMP( _ctx, 1 );

    if( dst == JS_NULLPTR )
    {
        return;
    }

    *(dst) = _value;
}
//////////////////////////////////////////////////////////////////////////
static js_result_t __js_dump_string( js_dump_ctx_t * _ctx, js_string_t _value )
{
    const char * value_str = _value.value;
    js_size_t origin_size = _value.size;
    js_size_t value_size = _value.size;

    for( const char * it_value = value_str,
        *it_value_end = value_str + origin_size;
        it_value != it_value_end; ++it_value )
    {
        char c = *it_value;

        switch( c )
        {
        case '\"':
            ++value_size;
            break;
        case '\\':
            ++value_size;
            break;
        default:
            break;
        }
    }

    char * dst = JS_DUMP( _ctx, value_size );

    if( dst == JS_NULLPTR )
    {
        return JS_FAILURE;
    }

    char * it_buffer = dst;

    for( const char * it_value = value_str,
        *it_value_end = value_str + origin_size;
        it_value != it_value_end; ++it_value )
    {
        char c = *it_value;

        switch( c )
        {
        case '\"':
            *it_buffer++ = '\\';
            *it_buffer++ = '\"';
            break;
        case '\\':
            *it_buffer++ = '\\';
            *it_buffer++ = '\\';
            break;
        default:
            *it_buffer++ = c;
        }
    }

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __js_dump_string_internal( js_dump_ctx_t * _ctx, const char * _value, js_size_t _size )
{
    js_string_t str = {_value, _size};

    __js_dump_string( _ctx, str );
}
//////////////////////////////////////////////////////////////////////////
#define JS_DUMP_INTERNAL(data, value) __js_dump_string_internal(data, value, sizeof( value ) - 1)
//////////////////////////////////////////////////////////////////////////
#define JS_MAX_INTEGER_SYMBOLS 20
//////////////////////////////////////////////////////////////////////////
static void __js_dump_integer( js_dump_ctx_t * _ctx, js_integer_t _value )
{
    if( _value == 0 )
    {
        JS_DUMP_INTERNAL( _ctx, "0" );

        return;
    }
    else if( _value == 1 )
    {
        JS_DUMP_INTERNAL( _ctx, "1" );

        return;
    }

    char symbols[JS_MAX_INTEGER_SYMBOLS] = {'\0'};

    char * it = symbols + JS_MAX_INTEGER_SYMBOLS;
    
    uint64_t value_u = (_value >= 0) ? (uint64_t)_value : (uint64_t)(-(_value + 1)) + 1;

    while( value_u )
    {
        uint64_t symbol = value_u % 10;
        value_u /= 10;

        *--it = '0' + (char)symbol;
    }

    if( _value < 0 )
    {
        *--it = '-';
    }

    js_size_t symbols_size = JS_MAX_INTEGER_SYMBOLS - (it - symbols);

    char * dst = JS_DUMP( _ctx, symbols_size );

    if( dst == JS_NULLPTR )
    {
        return;
    }

    js_memcpy( dst, it, symbols_size );
}
//////////////////////////////////////////////////////////////////////////
static void __js_dump_double( js_dump_ctx_t * _ctx, double _value, int32_t _precision )
{
    if( _value == 0.0 )
    {
        JS_DUMP_INTERNAL( _ctx, "0.0" );

        return;
    }
    else if( _value == 0.5 )
    {
        JS_DUMP_INTERNAL( _ctx, "0.5" );

        return;
    }
    else if( _value == 1.0 )
    {
        JS_DUMP_INTERNAL( _ctx, "1.0" );

        return;
    }

    if( _value < 0.0 )
    {
        _value = -_value;

        __js_dump_char( _ctx, '-' );
    }

    double r = 0.0000000000000005;
    double fr = _value + r;

    int64_t i = (int64_t)fr;
    fr -= (double)i;

    if( i == 0 )
    {
        __js_dump_char( _ctx, '0' );
    }
    else
    {
        js_size_t n = 0;
        
        int64_t in = i;
        while( in != 0 )
        {
            in /= 10;
            ++n;
        }

        char * dst = JS_DUMP( _ctx, n );

        if( dst == JS_NULLPTR )
        {
            return;
        }

        char * p = dst + n;

        while( n-- )
        {
            int64_t symbol = i % 10;
            i /= 10;

            *--p = '0' + (char)symbol;
        }
    }

    __js_dump_char( _ctx, '.' );

    if( fr == 0.0 )
    {
        __js_dump_char( _ctx, '0' );        
    }
    else
    {
        js_size_t n = 0;
        js_size_t nz = 0;

        double frn = fr;

        while( _precision-- )
        {
            frn *= 10.0;

            char c = (char)frn;
            frn -= c;

            ++n;

            if( c != 0 )
            {
                nz = n;
            }
        }

        if( nz == 0 )
        {
            __js_dump_char( _ctx, '0' );
        }
        else
        {
            char * dst = JS_DUMP( _ctx, nz );

            if( dst == JS_NULLPTR )
            {
                return;
            }

            while( nz-- )
            {
                fr *= 10.0;

                char c = (char)fr;
                fr -= c;

                *dst++ = '0' + c;
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static void __js_dump_element( js_dump_ctx_t * _ctx, const js_element_t * _element );
//////////////////////////////////////////////////////////////////////////
static void __js_dump_array_element( js_size_t _index, const js_element_t * _value, void * _ud )
{
    js_dump_ctx_t * ctx = (js_dump_ctx_t *)_ud;

    if( _index != 0 )
    {
        __js_dump_char( ctx, ',' );
    }

    __js_dump_element( ctx, _value );
}
//////////////////////////////////////////////////////////////////////////
static void __js_dump_array( js_dump_ctx_t * _ctx, const js_element_t * _element )
{
    __js_dump_char( _ctx, '[' );
    js_array_foreach( _element, &__js_dump_array_element, _ctx );
    __js_dump_char( _ctx, ']' );
}
//////////////////////////////////////////////////////////////////////////
static void __js_dump_object_element( js_size_t _index, const js_element_t * _key, const js_element_t * _value, void * _ud )
{
    js_dump_ctx_t * ctx = (js_dump_ctx_t *)_ud;

    if( _index != 0 )
    {
        __js_dump_char( ctx, ',' );
    }

    __js_dump_char( ctx, '"' );

    js_string_t key;
    js_get_string( _key, &key );

    __js_dump_string( ctx, key );
    __js_dump_char( ctx, '"' );

    __js_dump_char( ctx, ':' );

    __js_dump_element( ctx, _value );
}
//////////////////////////////////////////////////////////////////////////
static void __js_dump_object( js_dump_ctx_t * _ctx, const js_element_t * _element )
{
    __js_dump_char( _ctx, '{' );
    js_object_foreach( _element, &__js_dump_object_element, _ctx );
    __js_dump_char( _ctx, '}' );
}
//////////////////////////////////////////////////////////////////////////
static void __js_dump_element( js_dump_ctx_t * _ctx, const js_element_t * _element )
{
    js_type_t type = js_type( _element );

    switch( type )
    {
    case js_type_null:
        {
            JS_DUMP_INTERNAL( _ctx, "null" );
        }break;
    case js_type_false:
        {
            JS_DUMP_INTERNAL( _ctx, "false" );
        }break;
    case js_type_true:
        {
            JS_DUMP_INTERNAL( _ctx, "true" );
        }break;
    case js_type_integer:
        {
            js_integer_t value = js_get_integer( _element );

            __js_dump_integer( _ctx, value );
        }break;
    case js_type_real:
        {
            js_real_t value = js_get_real( _element );

            __js_dump_double( _ctx, value, 6 );
        }break;
    case js_type_string:
        {
            js_string_t str;
            js_get_string( _element, &str );

            __js_dump_char( _ctx, '"' );
            __js_dump_string( _ctx, str );
            __js_dump_char( _ctx, '"' );
        }break;
    case js_type_array:
        {
            __js_dump_array( _ctx, _element );
        }break;
    case js_type_object:
        {
            __js_dump_object( _ctx, _element );
        }break;
    }
}
//////////////////////////////////////////////////////////////////////////
static void * __js_dump_buffer( js_size_t _size, void * _ud )
{
    js_buffer_t * buffer = (js_buffer_t *)_ud;

    if( buffer->memory + _size > buffer->end )
    {
        buffer->memory = buffer->end;

        return JS_NULLPTR;
    }

    uint8_t * new_buffer = buffer->memory;

    buffer->memory += _size;

    return new_buffer;
}
//////////////////////////////////////////////////////////////////////////
void js_make_dump_ctx_buffer( js_buffer_t * _buffer, js_dump_ctx_t * const _ctx )
{
    _ctx->buffer = &__js_dump_buffer;
    _ctx->ud = _buffer;
}
//////////////////////////////////////////////////////////////////////////
void js_make_dump_ctx_default( js_dump_buffer_fun_t _fun, void * _ud, js_dump_ctx_t * const _ctx )
{
    _ctx->buffer = _fun;
    _ctx->ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
js_result_t js_dump( const js_element_t * _element, js_dump_ctx_t * _ctx )
{
    __js_dump_object( _ctx, _element );

    char * dst = JS_DUMP( _ctx, 1 );

    if( dst == JS_NULLPTR )
    {
        return JS_FAILURE;
    }

    *dst = '\0';

    return JS_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
js_result_t js_dump_string( js_string_t _value, char * const _buffer, js_size_t _capacity, js_size_t * const _size )
{
    js_buffer_t dump_buff;
    js_make_buffer( _buffer, _capacity, &dump_buff );

    js_dump_ctx_t ctx;
    js_make_dump_ctx_buffer( &dump_buff, &ctx );

    js_result_t result = __js_dump_string( &ctx, _value );

    if( result != JS_SUCCESSFUL )
    {
        return JS_FAILURE;
    }

    if( _size != JS_NULLPTR )
    {
        *_size = js_get_buffer_size( &dump_buff );
    }

    return JS_SUCCESSFUL;

}
//////////////////////////////////////////////////////////////////////////