#ifndef JSON_CONFIG_H_
#define JSON_CONFIG_H_

#if !defined(NDEBUG)
#   define JS_DEBUG
#endif

#include <stddef.h>
#include <stdint.h>

#if defined(_MSC_VER)
#   pragma warning(disable: 4200)
#endif

#ifndef JS_LLONG_MAX
#define JS_LLONG_MAX 9223372036854775807LL
#endif

typedef uint32_t js_result_t;
typedef uint32_t js_bool_t;
typedef size_t js_size_t;

typedef int64_t js_integer_t;
typedef double js_real_t;

typedef struct js_string_t
{
    const char * value;
    js_size_t size;
} js_string_t;

#define JS_PP_CONCATENATE( A, B ) A##B

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

#ifndef JS_CONST_STRING
#define JS_CONST_STRING(S) { S, sizeof(S) - 1 }
#endif

#ifndef JS_MAKE_STRING
#define JS_MAKE_STRING(S, Z) { S, Z }
#endif

#ifndef JS_CODE_FILE
#define JS_CODE_FILE __FILE__
#endif

#ifndef JS_CODE_LINE
#define JS_CODE_LINE __LINE__
#endif

#ifndef JS_ALLOCATOR_MEMORY_CHECK_ENABLE
#   if defined(JS_DEBUG)
#       define JS_ALLOCATOR_MEMORY_CHECK_ENABLE 1
#   else
#       define JS_ALLOCATOR_MEMORY_CHECK_ENABLE 0
#   endif
#endif

typedef enum js_type_e
{
    js_type_null = 0,
    js_type_false = 1,
    js_type_true = 2,
    js_type_integer = 3,
    js_type_real = 4,
    js_type_string = 5,
    js_type_array = 6,
    js_type_object = 7,
} js_type_e;

typedef uint32_t js_type_t;

typedef enum js_flags_e
{
    js_flag_none = 0,
    js_flag_string_inplace = 1 << 0,
    js_flag_node_pool = 1 << 1,
} js_flags_e;

typedef uint32_t js_flags_t;

typedef struct js_element_t js_element_t;

typedef struct js_buffer_t
{
    uint8_t * begin;
    uint8_t * end;
    uint8_t * memory;
} js_buffer_t;

#endif
