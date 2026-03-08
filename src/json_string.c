#include "json_string.h"

//////////////////////////////////////////////////////////////////////////
js_bool_t js_isspace( char c )
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_isdigit( char c )
{
    return (c >= '0' && c <= '9');
}
//////////////////////////////////////////////////////////////////////////
void js_memcpy( char * _dst, const char * _src, js_size_t _size )
{
    while( _size-- )
    {
        *_dst++ = *_src++;
    }
}
//////////////////////////////////////////////////////////////////////////
js_size_t js_strlen( const char * _value )
{
    const char * it = _value;

    for( ; *it != '\0'; ++it );

    return it - _value;
}
//////////////////////////////////////////////////////////////////////////
const char * js_strpbrk( const char * _begin, const char * _end, const char * _str )
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
js_bool_t js_chrskip( char _ch, const char * _str )
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
const char * js_strskip( const char * _begin, const char * _end, const char * _str )
{
    const char * it = _begin;

    for( ; it != _end; ++it )
    {
        if( js_chrskip( *it, _str ) == JS_FALSE )
        {
            return it;
        }
    }

    return JS_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * js_strstr( const char * _begin, const char * _end, const char * _str )
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
const char * js_strchr( const char * _begin, const char * _end, char _ch )
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
js_bool_t js_strneq( const char * _s1, const char * _s2, js_size_t _n )
{
    for( js_size_t index = 0; index != _n; ++index )
    {
        if( _s1[index] == '\0' )
        {
            return JS_FALSE;
        }

        if( _s1[index] != _s2[index] )
        {
            return JS_FALSE;
        }
    }

    return _s1[_n] == '\0' ? JS_TRUE : JS_FALSE;
}
//////////////////////////////////////////////////////////////////////////
js_bool_t js_strzeq( js_string_t _s1, js_string_t _s2 )
{
    if( _s1.size != _s2.size )
    {
        return JS_FALSE;
    }

    for( js_size_t index = 0; index != _s1.size; ++index )
    {
        if( _s1.value[index] != _s2.value[index] )
        {
            return JS_FALSE;
        }
    }

    return JS_TRUE;
}
//////////////////////////////////////////////////////////////////////////
#define JS_STRTOLL_INCREASE_EOF() ++s; if( s == _end ) { *_it = _in; return 0; }
//////////////////////////////////////////////////////////////////////////
int64_t js_strtoll( const char * _in, const char * _end, const char ** _it )
{
    const char * s = _in;

    js_bool_t neg = JS_FALSE;
    js_bool_t any = JS_FALSE;

    while( js_isspace( *s ) == JS_TRUE )
    {
        JS_STRTOLL_INCREASE_EOF();
    }

    if( *s == '-' )
    {
        neg = JS_TRUE;

        JS_STRTOLL_INCREASE_EOF();
    }
    else if( *s == '+' )
    {
        JS_STRTOLL_INCREASE_EOF();
    }

    int64_t value = 0;

    for( ;; )
    {
        char c = *s;

        if( c == '\0' )
        {
            break;
        }

        if( js_isdigit( c ) == JS_FALSE )
        {
            break;
        }

        int64_t d = c - '0';

        if( value > JS_LLONG_MAX / 10 )
        {
            *_it = _in;

            return 0;
        }

        value = value * 10 + d;

        any = JS_TRUE;

        JS_STRTOLL_INCREASE_EOF();
    }

    if( neg == JS_TRUE )
    {
        value = -value;
    }

    if( any == JS_FALSE )
    {
        *_it = _in;

        return 0;
    }

    *_it = s;

    return value;
}
//////////////////////////////////////////////////////////////////////////
#define JS_STRTOD_INCREASE_EOF() ++s; if( s == _end ) { *_it = _in; return 0.0; }
//////////////////////////////////////////////////////////////////////////
double js_strtod( const char * _in, const char * _end, const char ** _it )
{
    const int32_t max_exponent = 511;
    const double pow10[] = {10., 100., 1.0e4, 1.0e8, 1.0e16, 1.0e32, 1.0e64, 1.0e128, 1.0e256};

    js_bool_t sign = JS_TRUE;
    js_bool_t exponent_sign = JS_TRUE;

    const char * s = _in;

    while( js_isspace( *s ) == JS_TRUE )
    {
        JS_STRTOD_INCREASE_EOF();
    }

    if( *s == '-' )
    {
        sign = JS_FALSE;

        JS_STRTOD_INCREASE_EOF();
    }
    else if( *s == '+' )
    {
        JS_STRTOD_INCREASE_EOF();
    }

    int32_t mantissa_size = 0;
    int32_t decimal_point = -1;

    for( ;; ++mantissa_size )
    {
        char c = *s;

        if( js_isdigit( c ) == JS_FALSE )
        {
            if( (c != '.') || (decimal_point >= 0) )
            {
                break;
            }

            decimal_point = mantissa_size;
        }

        JS_STRTOD_INCREASE_EOF();
    }

    const char * p_exponent = s;

    s -= mantissa_size;

    if( decimal_point < 0 )
    {
        decimal_point = mantissa_size;
    }
    else
    {
        --mantissa_size;
    }

    int32_t fractional_exponent = 0;

    if( mantissa_size > 18 )
    {
        fractional_exponent = decimal_point - 18;

        mantissa_size = 18;
    }
    else
    {
        fractional_exponent = decimal_point - mantissa_size;
    }

    if( mantissa_size == 0 )
    {
        *_it = _in;

        if( sign == JS_FALSE )
        {
            return -0.0;
        }

        return 0.0;
    }

    int32_t fraction1 = 0;
    for( ; mantissa_size > 9; mantissa_size -= 1 )
    {
        char c = *s;

        JS_STRTOD_INCREASE_EOF();

        if( c == '.' )
        {
            c = *s;

            JS_STRTOD_INCREASE_EOF();
        }

        int32_t d = c - '0';

        fraction1 = 10 * fraction1 + d;
    }

    int32_t fraction2 = 0;
    for( ; mantissa_size > 0; mantissa_size -= 1 )
    {
        char c = *s;

        JS_STRTOD_INCREASE_EOF();

        if( c == '.' )
        {
            c = *s;

            JS_STRTOD_INCREASE_EOF();
        }

        int32_t d = c - '0';

        fraction2 = 10 * fraction2 + d;
    }

    double fraction = (1.0e9 * fraction1) + fraction2;

    s = p_exponent;

    int32_t exponent = 0;

    if( (*s == 'E') || (*s == 'e') )
    {
        JS_STRTOD_INCREASE_EOF();

        if( *s == '-' )
        {
            exponent_sign = JS_FALSE;

            JS_STRTOD_INCREASE_EOF();
        }
        else if( *s == '+' )
        {
            JS_STRTOD_INCREASE_EOF();
        }

        while( js_isdigit( *s ) == JS_TRUE )
        {
            int32_t d = *s - '0';

            exponent = exponent * 10 + d;

            JS_STRTOD_INCREASE_EOF();
        }
    }

    if( exponent_sign == JS_FALSE )
    {
        exponent = fractional_exponent - exponent;
    }
    else
    {
        exponent = fractional_exponent + exponent;
    }

    if( exponent < 0 )
    {
        exponent_sign = JS_FALSE;

        exponent = -exponent;
    }
    else
    {
        exponent_sign = JS_TRUE;
    }

    if( exponent > max_exponent )
    {
        *_it = _in;

        return 0.0;
    }

    double double_exponent = 1.0;

    for( const double * d = pow10; exponent != 0; exponent >>= 1, d += 1 )
    {
        if( exponent & 01 )
        {
            double value = *d;

            double_exponent *= value;
        }
    }

    if( exponent_sign == JS_FALSE )
    {
        fraction /= double_exponent;
    }
    else
    {
        fraction *= double_exponent;
    }

    *_it = s;

    if( sign == JS_FALSE )
    {
        return -fraction;
    }

    return fraction;
}
//////////////////////////////////////////////////////////////////////////