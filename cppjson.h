#ifndef INC_CPPJSON_H_
#define INC_CPPJSON_H_
// clang-format off
/*
# License
This software is distributed under two licenses, choose whichever you like.

## MIT License
Copyright (c) 2022 Takuro Sakai

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Public Domain
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/
// clang-format on
/**
@author t-sakai
*/
#include <cassert>
#include <cstdint>
#include <limits>

namespace cppjson
{
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

using std::uintptr_t;

#ifndef CPPJSON_NULL
#    ifdef __cplusplus
#        if 201103L <= __cplusplus || 1700 <= _MSC_VER
#            define CPPJSON_NULL nullptr
#        else
#            define CPPJSON_NULL 0
#        endif
#    else
#        define CPPJSON_NULL (void*)0
#    endif
#endif

#ifdef _DEBUG
#    define CPPJSON_ASSERT(exp) assert(exp)
#else
#    define CPPJSON_ASSERT(exp)
#endif

typedef void* (*cppjson_malloc)(size_t);
typedef void (*cppjson_free)(void*);

static constexpr u32 DefaultMaxNest = 512;
static constexpr u32 Invalid = 0xFFFFFFFFU;

class JsonParser;

/**
 * @brief Json native type
*/
enum class JsonType
{
    None,
    Object,
    Array,
    String,
    Number,
    Integer,
    Boolean,
    Null,
    KeyValue,
};

/**
 * @brief alternative of std::optional
*/
struct JsonResult
{
    bool success_;
    u32 index_;
};

struct JsonObject
{
    u32 size_;
    u32 head_;
};

struct JsonArray
{
    u32 size_;
    u32 head_;
};

struct JsonKeyValue
{
    u32 key_;
    u32 value_;
};

/**
 * @brief Base of all of json elements

All of json elemens are memoried as JsonValue. 
*/
struct JsonValue
{
    u32 position_;
    u32 length_;
    u32 next_;
    JsonType type_;
    union
    {
        JsonObject object_;
        JsonArray array_;
        JsonKeyValue key_value_;
    };
};

class JsonObjectProxy;
class JsonArrayProxy;

//--- JsonStringProxy
class JsonStringProxy
{
public:
    operator bool() const;
    s32 compare(u32 length, const char* str) const;
    u32 length() const;
    const char* str() const;

    const JsonParser* parent_;
    u32 element_;
};

//--- JsonValueProxy
class JsonValueProxy
{
public:
    operator bool() const;
    JsonType type() const;
    s32 asInt() const;
    s32 asInt(s32 defaultValue) const;
    u32 asUInt() const;
    f32 asFloat() const;
    f64 asDouble() const;
    JsonStringProxy asString() const;
    bool asBool() const;
    JsonObjectProxy asObject() const;
    JsonArrayProxy asArray() const;

    template<class T>
    T as() const;

    const JsonParser* parent_;
    u32 element_;
};

template<class T>
T JsonValueProxy::as() const
{
    if(Invalid != element_) {
        return {parent_, element_};
    }
    return {CPPJSON_NULL, Invalid};
}

//--- JsonKeyVlaueProxy
class JsonKeyVlaueProxy
{
public:
    operator bool() const;
    JsonStringProxy key() const;
    JsonValueProxy value() const;
    const JsonParser* parent_;
    u32 element_;
};

//--- JsonObjectProxy
class JsonObjectProxy
{
public:
    operator bool() const;

    u32 size() const;

    u32 existsKey(u32 length, const char* key) const;
    u32 existsKey(u32 length, const char* key, JsonType type) const;

    bool tryGetValue(JsonValueProxy& value, u32 length, const char* key) const;
    bool tryGetValue(JsonValueProxy& value, JsonType type, u32 length, const char* key) const;

    JsonValueProxy getValue(u32 length, const char* key) const;
    JsonStringProxy getAsString(u32 length, const char* key) const;
    template<class T>
    T getAs(u32 length, const char* key) const;

    template<class T>
    T getAs(u32 length, const char* key, T defaultValue) const;
    template<>
    s32 getAs<s32>(u32 length, const char* key, s32 defaultValue) const;
    template<>
    f32 getAs<f32>(u32 length, const char* key, f32 defaultValue) const;
    template<>
    bool getAs<bool>(u32 length, const char* key, bool defaultValue) const;

    const JsonParser* parent_;
    u32 element_;
};

template<class T>
T JsonObjectProxy::getAs(u32 length, const char* key) const
{
    CPPJSON_ASSERT(CPPJSON_NULL != key);
    u32 index = existsKey(length, key);
    if(Invalid != index) {
        return {parent_, (*parent_)[index].key_value_.value_};
    } else {
        return {CPPJSON_NULL, Invalid};
    }
}

template<class T>
T JsonObjectProxy::getAs(u32 length, const char* key, T defaultValue) const
{
}

//--- JsonArrayIterator
struct JsonArrayIterator
{
    bool next();
    JsonValueProxy current() const;
    const JsonParser* parent_;
    u32 element_;
    u32 current_;
};

//--- JsonArrayProxy
class JsonArrayProxy
{
public:
    operator bool() const;
    bool isHomogeneous(JsonType type) const;

    u32 size() const;
    JsonValueProxy operator[](u32 index) const;
    JsonArrayIterator begin() const;

    const JsonParser* parent_;
    u32 element_;
};

//--- JsonParser
//---------------------------------------
class JsonParser
{
public:
    using cursor = const char*;

    JsonParser(cppjson_malloc allocator = CPPJSON_NULL, cppjson_free deallocator = CPPJSON_NULL, u32 max_nests = DefaultMaxNest);
    ~JsonParser();

    bool parse(cursor head, cursor end);
    inline bool parse(const void* head, size_t size)
    {
        return parse(reinterpret_cast<cursor>(head), reinterpret_cast<cursor>(head) + size);
    }

    u32 size() const;
    JsonValueProxy getRoot() const;
    const JsonValue& operator[](u32 index) const;
    cursor row_data(u32 index) const;

private:
    JsonParser(const JsonParser&) = delete;
    JsonParser& operator=(const JsonParser&) = delete;

    struct Buffer
    {
    public:
        static constexpr u32 ExpandSize = 512;

        Buffer(cppjson_malloc allocator = CPPJSON_NULL, cppjson_free deallocator = CPPJSON_NULL);
        ~Buffer();

        u32 capacity() const;
        u32 size() const;
        void clear();
        void push_back(const JsonValue& value);

        const JsonValue& operator[](u32 index) const;
        JsonValue& operator[](u32 index);

    private:
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        cppjson_malloc allocator_;
        cppjson_free deallocator_;
        u32 capacity_;
        u32 size_;
        JsonValue* values_;
    };

    void add_element_to_object(u32 object_index, u32 element_index);
    void add_element_to_array(u32 array_index, u32 element_index);

    void skip_bom();
    void skip_space();
    JsonResult parse_value();
    JsonResult parse_object();
    JsonResult parse_array();
    JsonResult parse_string();
    JsonResult parse_true();
    JsonResult parse_false();
    JsonResult parse_null();
    JsonResult parse_number();

    s32 parse_escape();
    s32 parse_utf8();
    s32 parse_4hexdigit();
    void parse_integer();
    s32 parse_fraction();
    s32 parse_exponent();
    u32 get_position() const;
    JsonResult get_number(u32 begin, JsonType type);
    static bool is_hex_digit(s32 c);

    cppjson_malloc allocator_;
    cppjson_free deallocator_;
    u32 max_nests_;
    u32 nest_count_;
    cursor begin_ = CPPJSON_NULL;
    cursor current_ = CPPJSON_NULL;
    cursor end_ = CPPJSON_NULL;
    Buffer buffer_;
};

} // namespace cppjson
#endif // INC_CPPJSON_H_
