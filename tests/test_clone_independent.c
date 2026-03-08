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
    const char json[] = "{\"value\":10}";

    js_allocator_t allocator;
    js_make_allocator_default(&test_alloc, &test_free, JS_NULLPTR, &allocator);

    js_element_t * base = JS_NULLPTR;
    CHECK(js_parse(allocator, js_flag_none, json, sizeof(json), &on_failed, JS_NULLPTR, &base) == JS_SUCCESSFUL);

    js_element_t * clone = JS_NULLPTR;
    CHECK(js_clone(allocator, js_flag_none, base, &clone) == JS_SUCCESSFUL);

    js_string_t key_extra = JS_CONST_STRING("extra");
    CHECK(js_object_add_field_integer(clone, clone, key_extra, 99) == JS_SUCCESSFUL);

    js_element_t * clone_extra = js_object_get(clone, "extra");
    CHECK(clone_extra != JS_NULLPTR);
    CHECK(js_is_integer(clone_extra) == JS_TRUE);
    CHECK(js_get_integer(clone_extra) == 99);

    js_element_t * base_extra = js_object_get(base, "extra");
    CHECK(base_extra == JS_NULLPTR);

    js_free(base);
    js_free(clone);

    return EXIT_SUCCESS;
}
