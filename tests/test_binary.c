#include "json/json.h"
#include "json/json_binary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct binary_capture_t
{
    unsigned char data[512];
    js_size_t size;
} binary_capture_t;

static void * test_alloc(js_size_t size, void * ud)
{
    JS_UNUSED(ud);
    return malloc(size);
}

static void test_free(void * ptr, void * ud)
{
    JS_UNUSED(ud);
    free(ptr);
}

static void on_failed(const char * pointer, const char * end, const char * message, void * ud)
{
    JS_UNUSED(pointer);
    JS_UNUSED(end);
    JS_UNUSED(ud);

    printf("parse failed: %s\n", message);
}

static void capture_write(const void * buffer, js_size_t size, void * ud)
{
    binary_capture_t * capture = (binary_capture_t *)ud;

    if( capture->size + size > sizeof(capture->data) )
    {
        size = sizeof(capture->data) - capture->size;
    }

    memcpy(capture->data + capture->size, buffer, size);
    capture->size += size;
}

#define CHECK(EXPR) \
    do \
    { \
        if( !(EXPR) ) \
        { \
            printf("CHECK FAILED: %s\n", #EXPR); \
            return EXIT_FAILURE; \
        } \
    } while(0)

int main(void)
{
    const char json_non_empty[] = "{\"a\":1,\"b\":0.0,\"c\":\"x\"}";
    const char json_empty[] = "{}";

    js_allocator_t allocator;
    js_make_allocator_default(&test_alloc, &test_free, JS_NULLPTR, &allocator);

    js_element_t * non_empty = JS_NULLPTR;
    CHECK(js_parse(allocator, js_flag_none, json_non_empty, sizeof(json_non_empty), &on_failed, JS_NULLPTR, &non_empty) == JS_SUCCESSFUL);

    binary_capture_t capture_non_empty;
    memset(&capture_non_empty, 0, sizeof(capture_non_empty));

    js_binary_ctx_t ctx_non_empty;
    ctx_non_empty.write = &capture_write;
    ctx_non_empty.ud = &capture_non_empty;

    js_binary(non_empty, &ctx_non_empty);

    /* Root object with fields should produce non-empty binary payload. */
    CHECK(capture_non_empty.size > 2);
    CHECK(capture_non_empty.data[0] == 14u);
    CHECK(capture_non_empty.data[1] == 3u);

    js_free(non_empty);

    js_element_t * empty = JS_NULLPTR;
    CHECK(js_parse(allocator, js_flag_none, json_empty, sizeof(json_empty), &on_failed, JS_NULLPTR, &empty) == JS_SUCCESSFUL);

    binary_capture_t capture_empty;
    memset(&capture_empty, 0, sizeof(capture_empty));

    js_binary_ctx_t ctx_empty;
    ctx_empty.write = &capture_write;
    ctx_empty.ud = &capture_empty;

    js_binary(empty, &ctx_empty);

    /* Current implementation skips output for empty root object. */
    CHECK(capture_empty.size == 0);

    js_free(empty);

    return EXIT_SUCCESS;
}
