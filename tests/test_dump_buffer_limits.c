#include "json/json.h"
#include "json/json_dump.h"

#include <stdio.h>
#include <stdlib.h>

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
    const char json[] = "{\"name\":\"abcdef\"}";

    js_allocator_t allocator;
    js_make_allocator_default(&test_alloc, &test_free, JS_NULLPTR, &allocator);

    js_element_t * document = JS_NULLPTR;
    CHECK(js_parse(allocator, js_flag_none, json, sizeof(json), &on_failed, JS_NULLPTR, &document) == JS_SUCCESSFUL);

    char tiny_buffer[4];
    js_buffer_t buffer;
    js_make_buffer(tiny_buffer, sizeof(tiny_buffer), &buffer);

    js_dump_ctx_t dump_ctx;
    js_make_dump_ctx_buffer(&buffer, &dump_ctx);

    CHECK(js_dump(document, &dump_ctx) == JS_FAILURE);

    js_free(document);

    return EXIT_SUCCESS;
}
