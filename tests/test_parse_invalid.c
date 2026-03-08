#include "json/json.h"

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

typedef struct failed_ctx_t
{
    int called;
} failed_ctx_t;

static void on_failed(const char * pointer, const char * end, const char * message, void * ud)
{
    JS_UNUSED(pointer);
    JS_UNUSED(end);
    JS_UNUSED(message);

    failed_ctx_t * ctx = (failed_ctx_t *)ud;
    ctx->called = 1;
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
    const char invalid_json[] = "{\"a\":1";

    js_allocator_t allocator;
    js_make_allocator_default(&test_alloc, &test_free, JS_NULLPTR, &allocator);

    failed_ctx_t failed_ctx;
    failed_ctx.called = 0;

    js_element_t * document = JS_NULLPTR;
    js_result_t result = js_parse(
        allocator,
        js_flag_none,
        invalid_json,
        sizeof(invalid_json),
        &on_failed,
        &failed_ctx,
        &document
    );

    CHECK(result == JS_FAILURE);
    CHECK(failed_ctx.called == 1);
    CHECK(document == JS_NULLPTR);

    return EXIT_SUCCESS;
}
