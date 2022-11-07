# Introduction
[![Build Status](https://travis-ci.org/taqu/cppjson.svg?branch=master)](https://travis-ci.org/taqu/cppjson)

A simple json parser.

# Usage
To create the implementation, put '#define CPPJSON_IMPLEMENTATION' before including "cppjson.h".  
cppjson never provide or build structures like dictionaries, arrays, and so on. It has hierarchy information of Json's elements as a pair of it's position and size. 
You have to decode values, construct structures, if they will be needed.

<details>
<summary>Example</summary>

```cpp
#define CPPJSON_IMPLEMENTATION
#include "cppjson.h"

void traverse_object(cppjson::JsonProxy proxy);
void traverse_array(cppjson::JsonProxy proxy);
void traverse_keyvalue(cppjson::JsonProxy proxy);
void traverse_arrayvalue(cppjson::JsonProxy proxy);
void traverse_string(cppjson::JsonProxy proxy);
void traverse_number(cppjson::JsonProxy proxy);
void traverse_integer(cppjson::JsonProxy proxy);
void traverse_true(cppjson::JsonProxy proxy);
void traverse_false(cppjson::JsonProxy proxy);
void traverse_null(cppjson::JsonProxy proxy);

void traverse(cppjson::JsonProxy proxy)
{
    using namespace cppjson;
    switch(proxy.type()) {
    case JsonType::Object:
        traverse_object(proxy);
        break;
    case JsonType::Array:
        traverse_array(proxy);
        break;
    case JsonType::KeyValue:
        traverse_keyvalue(proxy);
        break;
    case JsonType::ArrayValue:
        traverse_arrayvalue(proxy);
        break;
    case JsonType::String:
        traverse_string(proxy);
        break;
    case JsonType::Number:
        traverse_number(proxy);
        break;
    case JsonType::Integer:
        traverse_integer(proxy);
        break;
    case JsonType::True:
        traverse_true(proxy);
        break;
    case JsonType::False:
        traverse_false(proxy);
        break;
    case JsonType::Null:
        traverse_null(proxy);
        break;
    }
}

void traverse_object(cppjson::JsonProxy proxy)
{
    using namespace cppjson;
    printf("{");
    for (JsonProxy i = proxy.begin(); i; i = i.next()) {
        JsonType type = i.type();
        char key[128];
        i.key().getString(key);
        printf("%s: ", key);
        traverse(i.value());
        printf(", ");
    }
    printf("}");
}

void traverse_array(cppjson::JsonProxy proxy)
{
    using namespace cppjson;
    printf("[");
    for(JsonProxy i = proxy.begin(); i; i = i.next()) {
        traverse(i);
        printf(", ");
    }
    printf("]");
}

void traverse_keyvalue(cppjson::JsonProxy proxy)
{
    char key[128];
    proxy.key().getString(key);
    printf("%s: ", key);
    traverse(proxy.value());
}

void traverse_arrayvalue(cppjson::JsonProxy proxy)
{
    traverse(proxy.value());
}

void traverse_string(cppjson::JsonProxy proxy)
{
    //correct way to acuire a string
    uint64_t length = proxy.size();
    char* value = reinterpret_cast<char*>(::malloc(length+1));
    if(NULL == value){
        return;
    }
    proxy.getString(value);
    printf("%s", value);
    ::free(value);
}

void traverse_number(cppjson::JsonProxy proxy)
{
    double value = proxy.getFloat64();
    printf("%lf", value);
}

void traverse_integer(cppjson::JsonProxy proxy)
{
    int64_t value = proxy.getInt64();
    printf("%lld", value);
}

void traverse_true(cppjson::JsonProxy proxy)
{
    printf("true");
}

void traverse_false(cppjson::JsonProxy proxy)
{
    printf("false");
}

void traverse_null(cppjson::JsonProxy proxy)
{
    printf("null");
}

void test(const char* path)
{
    FILE* f = fopen(path, "rb");
    if(NULL == f) {
        return;
    }
    struct stat s;
    fstat(fileno(f), &s);
    size_t size = s.st_size;
    char* data = (char*)::malloc(size);
    if(NULL == data || fread(data, size, 1, f) <= 0) {
        fclose(f);
        ::free(data);
        return;
    }
    fclose(f);
    cppjson::JsonReader reader;
    bool result = reader.parse(data, data + size);
    assert(result);
    cppjson::JsonProxy proxy = reader.root();
    traverse(proxy);
    ::free(data);
}

int main(void)
{
    test("../test00.json");
	return 0;
}
```

</details>

# License
This software is distributed under two licenses, MIT License or Public Domain, choose whichever you like.

