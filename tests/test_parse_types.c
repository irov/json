#include "json/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    const char json[] = "{\"count\":42,\"price\":3.5,\"name\":\"box\",\"enabled\":true,\"items\":[1,2],\"meta\":{\"version\":1}}";

    js_allocator_t allocator;
    js_make_allocator_default(&test_alloc, &test_free, JS_NULLPTR, &allocator);

    js_element_t * document = JS_NULLPTR;
    js_result_t result = js_parse(
        allocator,
        js_flag_none,
        json,
        sizeof(json),
        &on_failed,
        JS_NULLPTR,
        &document
    );

    CHECK(result == JS_SUCCESSFUL);
    CHECK(document != JS_NULLPTR);

    js_element_t * count = js_object_get(document, "count");
    CHECK(count != JS_NULLPTR);
    CHECK(js_is_integer(count) == JS_TRUE);
    CHECK(js_get_integer(count) == 42);

    js_element_t * price = js_object_get(document, "price");
    CHECK(price != JS_NULLPTR);
    CHECK(js_is_real(price) == JS_TRUE);
    CHECK(js_get_real(price) > 3.49 && js_get_real(price) < 3.51);

    js_element_t * name = js_object_get(document, "name");
    CHECK(name != JS_NULLPTR);
    CHECK(js_is_string(name) == JS_TRUE);

    js_string_t value_name;
    js_get_string(name, &value_name);
    CHECK(value_name.size == 3);
    CHECK(strncmp(value_name.value, "box", value_name.size) == 0);

    js_element_t * enabled = js_object_get(document, "enabled");
    CHECK(enabled != JS_NULLPTR);
    CHECK(js_is_boolean(enabled) == JS_TRUE);
    CHECK(js_get_boolean(enabled) == JS_TRUE);

    js_element_t * items = js_object_get(document, "items");
    CHECK(items != JS_NULLPTR);
    CHECK(js_is_array(items) == JS_TRUE);
    CHECK(js_array_size(items) == 2);

    js_element_t * meta = js_object_get(document, "meta");
    CHECK(meta != JS_NULLPTR);
    CHECK(js_is_object(meta) == JS_TRUE);

    js_free(document);

    return EXIT_SUCCESS;
}
