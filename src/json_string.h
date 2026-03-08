#ifndef JSON_STRING_H_
#define JSON_STRING_H_

#include "json/json_config.h"

js_bool_t js_isspace( char c );
js_bool_t js_isdigit( char c );
void js_memcpy( char * _dst, const char * _src, js_size_t _size );
js_size_t js_strlen( const char * _value );
const char * js_strpbrk( const char * _begin, const char * _end, const char * _str );
js_bool_t js_chrskip( char _ch, const char * _str );
const char * js_strskip( const char * _begin, const char * _end, const char * _str );
const char * js_strstr( const char * _begin, const char * _end, const char * _str );
const char * js_strchr( const char * _begin, const char * _end, char _ch );
js_bool_t js_strneq( const char * _s1, const char * _s2, js_size_t _n );
js_bool_t js_strzeq( js_string_t _s1, js_string_t _s2 );
int64_t js_strtoll( const char * _in, const char * _end, const char ** _it );
double js_strtod( const char * _in, const char * _end, const char ** _it );

#endif