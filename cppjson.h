#ifndef INC_CPPJSON_H_
#define INC_CPPJSON_H_
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
/**
@file cppjson.h

USAGE:
  put '#define CPPJSON_IMPLEMENTATION' before including this file to create the implementation.

*/
#include <cassert>
#include <cstddef>
#include <cstdint>

#include <tuple>

namespace cppjson
{
#ifndef CPPJSON_TYPES
#    define CPPJSON_TYPES
#    ifndef CPPJSON_NULL
#        define CPPJSON_NULL nullptr
#    endif // CPPJSON_NULL

#endif // CPPJSON_TYPES

typedef void* (*CPPJSON_MALLOC_TYPE)(size_t);
typedef void (*CPPJSON_FREE_TYPE)(void*);

#ifndef CPPJSON_ASSERT
#    define CPPJSON_ASSERT(exp) assert(exp)
#endif // CPPJSON_ASSERT

/**
 * @brief types
 */
enum class JsonType
{
    Object = 0,
    Array,
    KeyValue,
    ArrayValue,
    String,
    Number,
    Integer,
    True,
    False,
    Null,
    Invalid,
};

/**
 * @brief value type
 */
struct JsonValue
{
    uint64_t start_; //!< the start position of element
    uint64_t size_; //!< the size of element
    uint32_t next_; //!< the next element of aggretations
    uint32_t type_; //!< the type of element
};

/**
 * @brief Json element
 */
struct JsonProxy
{
    /**
     * @brief validate this element
     * @return true if this is valid
     */
    operator bool() const;

    /**
     * @return type of this
     */
    JsonType type() const;

    /**
     * Indicates the size of element. In a case of the string type, it shows the length of the string. For excample "test", the size equals to 4, excepts characters `"`.
     * @return size of this
     */
    uint64_t size() const;

    /**
     * In typical usage, you can iterate children,
     *
     * ```cpp
     * for(JsonProxy i=aggregation.begin(); i; i=i.next())
     * ```
     * @return the first element of aggrigations, like the object or array
     */
    JsonProxy begin() const;

    /**
     * @return the next element of aggrations
     */
    JsonProxy next() const;

    /**
     * @return key of an object's entry
     */
    JsonProxy key() const;
    /**
     * @return value of an object's entry
     */
    JsonProxy value() const;

    /**
     * @brief Get the value as string
     * @param [out] str ... the result
     * @return size of the result
     */
    uint64_t getString(char* str) const;

    /**
     * @brief Get the value as integer
     * @return the value as integer
     */
    int64_t getInt64() const;
    /**
     * @brief Get the value as float
     * @return the value as float
     */
    double getFloat64() const;

    bool compareKey(const char* str) const;

    uint64_t value_;
    const char* data_;
    const JsonValue* values_;
};

/**
 * @brief parser of a Json document
 */
class JsonReader
{
public:
    static constexpr uint32_t Invalid = static_cast<uint32_t>(-1); //!< Invalid value as uint32_t
    static constexpr std::tuple<const char*, uint32_t> InvalidPair = {CPPJSON_NULL, Invalid}; //!< Invalid value of the pair of next and value
    static constexpr uint32_t Expand = 128; //!< the expansion of buffer's capacity
    static constexpr int32_t MaxNesting = 128; //!< the maximum of nesting for objects or arrays

    /**
     * @param max_nesting ... the maximum of nesting for objects or arrays
     * @param alloc ... the function for memory allocation
     * @param dealloc ... the furnction for memory deallocation
     * @warning the alloc and dealloc must be passed simultaneously
     */
    JsonReader(int32_t max_nesting = MaxNesting, CPPJSON_MALLOC_TYPE alloc = CPPJSON_NULL, CPPJSON_FREE_TYPE dealloc = CPPJSON_NULL);
    ~JsonReader();

    /**
     * @param begin
     * @param end
     * @return
     * @pre begin != null
     * @pre end != null
     * @pre begin<=end
     */
    bool parse(const char* begin, const char* end);
    JsonProxy root() const;
private:
    JsonReader(const JsonReader&) = delete;
    JsonReader& operator=(const JsonReader&) = delete;

    uint32_t add();
    void add_value(uint32_t set, uint32_t value);

    const char* whitespace(const char* str);
    const char* parse_utf8(const char* str);
    const char* parse_element(const char* str);
    std::tuple<const char*, uint32_t> parse_value(const char* str);
    std::tuple<const char*, uint32_t> parse_string(const char* str);
    const char* parse_4hex(const char* str);
    const char* parse_zero_number(JsonType& type, const char* str);
    const char* parse_number(JsonType& type, const char* str);
    const char* parse_fraction(const char* str);
    const char* parse_exponent(const char* str);
    const char* parse_digits(const char* str);
    std::tuple<const char*, uint32_t> parse_object(const char* str);
    std::tuple<const char*, uint32_t> parse_member(const char* str);
    std::tuple<const char*, uint32_t> parse_array(const char* str);
    std::tuple<const char*, uint32_t> parse_array_value(const char* str);
    const char* parse_true(const char* str);
    const char* parse_false(const char* str);
    const char* parse_null(const char* str);

    CPPJSON_MALLOC_TYPE alloc_; //!< allocator
    CPPJSON_FREE_TYPE dealloc_; //!< deallocator
    const char* begin_; //!< begin of document
    const char* end_; //!< end of document
    int32_t max_nesting_; //!< the maximum of nesting
    int32_t nesting_; //!< current nesting

    uint32_t capacity_; //!< capacity of buffer
    uint32_t size_; //!< current size of buffer
    JsonValue* values_; //!< elements of Json
};
} // namespace cppjson

#endif // INC_CPPJSON_H_

#ifdef CPPJSON_IMPLEMENTATION
#include <charconv>
#include <cstdlib>
#include <cstring>

namespace cppjson
{

JsonProxy::operator bool() const
{
    return JsonReader::Invalid != value_;
}

JsonType JsonProxy::type() const
{
    if(JsonReader::Invalid != value_) {
        return static_cast<JsonType>(values_[value_].type_);
    }
    return JsonType::Invalid;
}

uint64_t JsonProxy::size() const
{
    CPPJSON_ASSERT(JsonReader::Invalid != value_);
    return values_[value_].size_;
}

JsonProxy JsonProxy::begin() const
{
    CPPJSON_ASSERT(JsonReader::Invalid != value_);
    if(static_cast<uint32_t>(JsonType::Object) != values_[value_].type_
       && static_cast<uint32_t>(JsonType::Array) != values_[value_].type_) {
        return {JsonReader::Invalid, CPPJSON_NULL, CPPJSON_NULL};
    }
    return {values_[value_].next_, data_, values_};
}

JsonProxy JsonProxy::next() const
{
    CPPJSON_ASSERT(JsonReader::Invalid != value_);
    return {values_[value_].next_, data_, values_};
}

JsonProxy JsonProxy::key() const
{
    CPPJSON_ASSERT(JsonReader::Invalid != value_);
    if(JsonType::KeyValue != type()) {
        return {JsonReader::Invalid, CPPJSON_NULL, CPPJSON_NULL};
    }
    return {values_[value_].start_, data_, values_};
}

JsonProxy JsonProxy::value() const
{
    CPPJSON_ASSERT(JsonReader::Invalid != value_);
    if(JsonType::KeyValue != type() && JsonType::ArrayValue != type()) {
        return {JsonReader::Invalid, CPPJSON_NULL, CPPJSON_NULL};
    }
    return {values_[value_].size_, data_, values_};
}

uint64_t JsonProxy::getString(char* str) const
{
    ::memcpy(str, data_ + values_[value_].start_, values_[value_].size_);
    str[values_[value_].size_] = '\0';
    return values_[value_].size_;
}

int64_t JsonProxy::getInt64() const
{
    const char* first = data_ + values_[value_].start_;
    const char* last = first + values_[value_].size_;
    int64_t value = 0;
    std::from_chars(first, last, value);
    return value;
}

double JsonProxy::getFloat64() const
{
    const char* first = data_ + values_[value_].start_;
#if _MSC_VER
    const char* last = first + values_[value_].size_;
    double value = 0;
    std::from_chars(first, last, value);
#else
    char buffer[127];
    uint64_t size = values_[value_].size_ < 128ULL ? values_[value_].size_ : 127ULL;
    ::memcpy(buffer, first, size);
    buffer[size] = '\0';
    double value = strtod(buffer, CPPJSON_NULL);
#endif
    return value;
}

bool JsonProxy::compareKey(const char* str) const
{
    CPPJSON_ASSERT(nullptr != str);
    CPPJSON_ASSERT(JsonReader::Invalid != value_);
    if(JsonType::KeyValue != type()) {
        return false;
    }
    return 0 == ::strncmp(str, data_ + values_[value_].start_, values_[value_].size_);
}

JsonReader::JsonReader(int32_t max_nesting, CPPJSON_MALLOC_TYPE alloc, CPPJSON_FREE_TYPE dealloc)
    : alloc_(alloc)
    , dealloc_(dealloc)
    , max_nesting_(max_nesting)
    , nesting_(0)
    , capacity_(0)
    , size_(0)
    , values_(CPPJSON_NULL)
{
    CPPJSON_ASSERT(0 < max_nesting_);
    if(CPPJSON_NULL == alloc_ || CPPJSON_NULL == dealloc_) {
        alloc_ = ::malloc;
        dealloc_ = ::free;
    }
    CPPJSON_ASSERT(CPPJSON_NULL != alloc_);
    CPPJSON_ASSERT(CPPJSON_NULL != dealloc_);
}

JsonReader::~JsonReader()
{
    dealloc_(values_);
    values_ = CPPJSON_NULL;
}

bool JsonReader::parse(const char* begin, const char* end)
{
    CPPJSON_ASSERT(CPPJSON_NULL != begin);
    CPPJSON_ASSERT(CPPJSON_NULL != end);
    CPPJSON_ASSERT(begin <= end);
    begin_ = begin;
    end_ = end;
    size_ = 0;

    const char* str = parse_element(begin_);
    if(CPPJSON_NULL == str) {
        return false;
    }
    str = whitespace(str);
    return end_ <= str;
}

JsonProxy JsonReader::root() const
{
    if(size_ <= 0) {
        return {Invalid, CPPJSON_NULL, CPPJSON_NULL};
    }
    return {0, begin_, values_};
}

uint32_t JsonReader::add()
{
    if(capacity_ <= size_) {
        uint32_t capacity = capacity_ + Expand;
        JsonValue* values = reinterpret_cast<JsonValue*>(alloc_(sizeof(JsonValue) * capacity));
        if(0 < capacity_) {
            ::memcpy(values, values_, sizeof(JsonValue) * capacity_);
        }
        dealloc_(values_);
        capacity_ = capacity;
        values_ = values;
    }
    uint32_t current = size_;
    ++size_;
    return current;
}

void JsonReader::add_value(uint32_t set, uint32_t value)
{
    ++values_[set].size_;
    while(Invalid != values_[set].next_) {
        set = values_[set].next_;
    }
    values_[set].next_ = value;
}

const char* JsonReader::whitespace(const char* str)
{
    while(str < end_) {
        switch(str[0]) {
        case 0x20:
        case 0x0A:
        case 0x0D:
        case 0x09:
            ++str;
            break;
        default:
            return str;
        }
    }
    return str;
}

const char* JsonReader::parse_utf8(const char* str)
{
    const uint8_t* u = reinterpret_cast<const uint8_t*>(str);

    if(0x20U <= u[0] && u[0] < 0x80U) {
        return str + 1;
    }

    if(0xC2U <= u[0] && u[0] <= 0xDFU) {
        if(end_ <= (str + 1)) {
            return CPPJSON_NULL;
        }
        return str + 2;
    }

    if(0xE0U <= u[0] && u[0] < 0xF0U) {
        if(end_ <= (str + 2)) {
            return CPPJSON_NULL;
        }
        if(0xE0U == u[0] && 0x80U <= u[1] && u[1] <= 0x9FU) {
            return CPPJSON_NULL;
        }
        return str + 3;
    }

    if(0xF0U <= u[0] && u[0] < 0xF4U) {
        if(end_ <= (str + 3)) {
            return CPPJSON_NULL;
        }
        if(0xF0U == u[0] && 0x80U <= u[1] && u[1] <= 0x8FU) {
            return CPPJSON_NULL;
        }
        if(0xF4U == u[0] && 0x90U <= u[1]) {
            return CPPJSON_NULL;
        }
        return str + 4;
    }
    return CPPJSON_NULL;
}

const char* JsonReader::parse_element(const char* str)
{
    str = whitespace(str);
    if(end_ <= str) {
        return CPPJSON_NULL;
    }
    auto [n, v] = parse_value(str);
    str = n;
    if(CPPJSON_NULL == str) {
        return CPPJSON_NULL;
    }
    return whitespace(str);
}

std::tuple<const char*, uint32_t> JsonReader::parse_value(const char* str)
{
    const char* begin = str;
    const char* next = CPPJSON_NULL;
    JsonType type;
    switch(str[0]) {
    case '"':
        return parse_string(str);
    case '-': {
        ++str;
        if(end_ <= str || str[0] < '0' || '9' < str[0]) {
            return InvalidPair;
        }
        switch(str[0]) {
        case '0': {
            next = parse_zero_number(type, str);
        } break;
        default:
            next = parse_number(type, str);
            break;
        }
    } break;
    case '0':
        next = parse_zero_number(type, str);
        break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        next = parse_number(type, str);
        break;
    case '{':
        return parse_object(str);
    case '[':
        return parse_array(str);
    case 't':
        type = JsonType::True;
        next = parse_true(str);
        break;
    case 'f':
        type = JsonType::False;
        next = parse_false(str);
        break;
    case 'n':
        type = JsonType::Null;
        next = parse_null(str);
        break;
    default:
        return InvalidPair;
    }
    if(CPPJSON_NULL == next) {
        return InvalidPair;
    }

    uint32_t value = add();
    values_[value].start_ = reinterpret_cast<uint64_t>(begin) - reinterpret_cast<uint64_t>(begin_);
    values_[value].size_ = reinterpret_cast<uint64_t>(next) - reinterpret_cast<uint64_t>(begin);
    values_[value].next_ = Invalid;
    values_[value].type_ = static_cast<uint32_t>(type);
    return {next, value};
}

std::tuple<const char*, uint32_t> JsonReader::parse_string(const char* str)
{
    CPPJSON_ASSERT('"' == str[0]);
    ++str;
    const char* begin = str;
    uint32_t value = add();
    values_[value].start_ = reinterpret_cast<uint64_t>(str) - reinterpret_cast<uint64_t>(begin_);
    values_[value].next_ = Invalid;
    values_[value].type_ = static_cast<uint32_t>(JsonType::String);
    while(str < end_) {
        switch(str[0]) {
        case '"':
            values_[value].size_ = reinterpret_cast<uint64_t>(str) - reinterpret_cast<uint64_t>(begin);
            return {str + 1, value};
        case '\\': {
            const char* next = str + 1;
            if(end_ <= next) {
                return InvalidPair;
            }
            switch(next[0]) {
            case '"':
            case '\\':
            case '/':
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
                str = next + 1;
                break;
            case 'u':
                str = parse_4hex(next + 1);
                if(CPPJSON_NULL == str) {
                    return InvalidPair;
                }
                ++str;
                break;
            default:
                return InvalidPair;
            }
        } break;
        default:
            str = parse_utf8(str);
            if(CPPJSON_NULL == str) {
                return InvalidPair;
            }
            break;
        }
    }
    return InvalidPair;
}

const char* JsonReader::parse_4hex(const char* str)
{
    uint32_t count = 0;
    while(str < end_) {
        switch(str[0]) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            ++count;
            break;
        default:
            return CPPJSON_NULL;
        }
        if(4 <= count) {
            return str;
        }
        ++str;
    }
    return CPPJSON_NULL;
}

const char* JsonReader::parse_zero_number(JsonType& type, const char* str)
{
    CPPJSON_ASSERT('0' == str[0]);
    ++str;
    if(end_ <= str) {
        return str;
    }
    switch(str[0]) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return CPPJSON_NULL;
    case '.':
        type = JsonType::Number;
        return parse_fraction(str);
    case 'e':
    case 'E':
        type = JsonType::Number;
        return parse_exponent(str);
    default:
        type = JsonType::Integer;
        return str;
    }
}

const char* JsonReader::parse_number(JsonType& type, const char* str)
{
    CPPJSON_ASSERT('1' <= str[0] && str[0] <= '9');
    ++str;
    while(str < end_) {
        switch(str[0]) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            ++str;
            break;
        case '.':
            type = JsonType::Number;
            return parse_fraction(str);
        case 'e':
        case 'E':
            type = JsonType::Number;
            return parse_exponent(str);
        default:
            type = JsonType::Integer;
            return str;
        }
    }
    type = JsonType::Integer;
    return str;
}

const char* JsonReader::parse_fraction(const char* str)
{
    CPPJSON_ASSERT('.' == str[0]);
    str = parse_digits(str + 1);
    if(CPPJSON_NULL == str || end_ <= str) {
        return str;
    }
    switch(str[0]) {
    case 'e':
    case 'E':
        return parse_exponent(str);
    default:
        return str;
    }
}

const char* JsonReader::parse_exponent(const char* str)
{
    CPPJSON_ASSERT('e' == str[0] || 'E' == str[0]);
    ++str;
    if(end_ <= str) {
        return CPPJSON_NULL;
    }
    if(str[0] == '-' || str[0] == '+') {
        ++str;
        if(end_ <= str) {
            return CPPJSON_NULL;
        }
    }
    return parse_digits(str);
}

const char* JsonReader::parse_digits(const char* str)
{
    uint32_t count = 0;
    while(str < end_) {
        switch(str[0]) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            ++str;
            ++count;
            break;
        default:
            return (0 < count) ? str : CPPJSON_NULL;
        }
    }
    return (0 < count) ? str : CPPJSON_NULL;
}

std::tuple<const char*, uint32_t> JsonReader::parse_object(const char* str)
{
    CPPJSON_ASSERT('{' == str[0]);
    if(max_nesting_ < ++nesting_) {
        return InvalidPair;
    }
    uint32_t object = add();
    values_[object].start_ = reinterpret_cast<uint64_t>(str) - reinterpret_cast<uint64_t>(begin_);
    values_[object].size_ = 0;
    values_[object].next_ = Invalid;
    values_[object].type_ = static_cast<uint32_t>(JsonType::Object);

    ++str;
    bool needs_member = false;
    bool needs_comma = false;
    while(str < end_) {
        str = whitespace(str);
        if(end_ <= str) {
            return InvalidPair;
        }
        switch(str[0]) {
        case '}':
            --nesting_;
            if(!needs_member) {
                return {str + 1, object};
            } else {
                return InvalidPair;
            }
        case '"': {
            if(needs_comma) {
                return InvalidPair;
            }
            auto [n, v] = parse_member(str);
            str = n;
            if(CPPJSON_NULL == str) {
                return InvalidPair;
            }
            add_value(object, v);
            needs_member = false;
            needs_comma = true;
        } break;
        case ',':
            if(needs_member) {
                return InvalidPair;
            }
            ++str;
            needs_member = true;
            needs_comma = false;
            break;
        default:
            return InvalidPair;
        }
    }
    return InvalidPair;
}

std::tuple<const char*, uint32_t> JsonReader::parse_member(const char* str)
{
    uint32_t keyvalue = add();
    values_[keyvalue].start_ = Invalid;
    values_[keyvalue].size_ = Invalid;
    values_[keyvalue].next_ = Invalid;
    values_[keyvalue].type_ = static_cast<uint32_t>(JsonType::KeyValue);

    auto [n0, v0] = parse_string(str);
    str = n0;
    if(CPPJSON_NULL == str) {
        return InvalidPair;
    }
    values_[keyvalue].start_ = v0;
    str = whitespace(str);
    if(end_ <= str || ':' != str[0]) {
        return InvalidPair;
    }
    str = whitespace(str + 1);
    if(end_ <= str) {
        return InvalidPair;
    }
    auto [n1, v1] = parse_value(str);
    values_[keyvalue].size_ = v1;
    return {n1, keyvalue};
}

std::tuple<const char*, uint32_t> JsonReader::parse_array(const char* str)
{
    CPPJSON_ASSERT('[' == str[0]);
    if(max_nesting_ < ++nesting_) {
        return InvalidPair;
    }
    uint32_t object = add();
    values_[object].start_ = reinterpret_cast<uint64_t>(str);
    values_[object].size_ = 0;
    values_[object].next_ = Invalid;
    values_[object].type_ = static_cast<uint32_t>(JsonType::Array);
    ++str;
    bool needs_value = false;
    bool needs_comma = false;
    while(str < end_) {
        str = whitespace(str);
        if(end_ <= str) {
            return InvalidPair;
        }
        switch(str[0]) {
        case ']':
            --nesting_;
            if(!needs_value) {
                return {str + 1, object};
            } else {
                return InvalidPair;
            }
        case ',':
            if(needs_value) {
                return InvalidPair;
            }
            ++str;
            needs_value = true;
            needs_comma = false;
            break;
        default:
            if(needs_comma) {
                return InvalidPair;
            }
            auto [n, v] = parse_array_value(str);
            str = n;
            if(CPPJSON_NULL == str) {
                return InvalidPair;
            }
            add_value(object, v);
            needs_value = false;
            needs_comma = true;
            break;
        }
    }
    return InvalidPair;
}

std::tuple<const char*, uint32_t> JsonReader::parse_array_value(const char* str)
{
    uint32_t arrayvalue = add();
    values_[arrayvalue].start_ = Invalid;
    values_[arrayvalue].size_ = Invalid;
    values_[arrayvalue].next_ = Invalid;
    values_[arrayvalue].type_ = static_cast<uint32_t>(JsonType::ArrayValue);

    auto [n0, v0] = parse_value(str);
    str = n0;
    if(CPPJSON_NULL == str) {
        return InvalidPair;
    }
    values_[arrayvalue].size_ = v0;
    return {n0, arrayvalue};
}

const char* JsonReader::parse_true(const char* str)
{
    CPPJSON_ASSERT('t' == str[0]);
    if(end_ <= (str + 3)) {
        return CPPJSON_NULL;
    }
    if('r' == str[1] && 'u' == str[2] && 'e' == str[3]) {
        return str + 4;
    }
    return CPPJSON_NULL;
}

const char* JsonReader::parse_false(const char* str)
{
    CPPJSON_ASSERT('f' == str[0]);
    if(end_ <= (str + 4)) {
        return CPPJSON_NULL;
    }
    if('a' == str[1] && 'l' == str[2] && 's' == str[3] && 'e' == str[4]) {
        return str + 5;
    }
    return CPPJSON_NULL;
}

const char* JsonReader::parse_null(const char* str)
{
    CPPJSON_ASSERT('n' == str[0]);
    if(end_ <= (str + 3)) {
        return CPPJSON_NULL;
    }
    if('u' == str[1] && 'l' == str[2] && 'l' == str[3]) {
        return str + 4;
    }
    return CPPJSON_NULL;
}

} // namespace cppjson
#endif // CPPJSON_IMPLEMENTATION
