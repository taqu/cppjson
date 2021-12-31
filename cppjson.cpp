// clang-format off
/*
# License
This software is distributed under two licenses, choose whichever you like.

## MIT License
Copyright (c) 2020-2021 Takuro Sakai

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
#include "cppjson.h"
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <limits>

namespace cppjson
{
namespace
{
    template<class T>
    T clamp(T x, T minx, T maxx)
    {
        CPPJSON_ASSERT(minx <= maxx);
        if(x < minx) {
            return minx;
        } else if(maxx < x) {
            return maxx;
        }
        return x;
    }

    static const char* JsonStrEmpty = "";

    //#if defined(__GNUC__) || defined(__clang__)
    //  __builtin_add_overflow
    //  __builtin_sub_overflow
    //  __builtin_mul_overflow
    //#elif defined(_MSC_VER)
    bool add_overflow(s32& r, s32 x0, s32 x1)
    {
        if((std::numeric_limits<s32>::max)() - x1 < x0) {
            return true;
        }
        r = x0 + x1;
        return false;
    }

    bool sub_overflow(s32& r, s32 x0, s32 x1)
    {
        if(x0 < (std::numeric_limits<s32>::min)() + x1) {
            return true;
        }
        r = x0 - x1;
        return false;
    }

    bool mul_overflow(s32& r, s32 x0)
    {
        s32 t = x0 * 10;
        if(x0 == (t / 10)) {
            r = t;
            return false;
        }
        return true;
    }

    /**
     * @brief Convert string to signed integer
     * @param str [in] ... string
     * @param len [in] ... length of str
     * @return The converted value. If the result is overflowed, INT_MAX or INT_MIN be returned.
     */
    s32 atoi(const char* str, u32 len)
    {
        CPPJSON_ASSERT(CPPJSON_NULL != str);
        CPPJSON_ASSERT(0 < len);
#ifdef _DEBUG
        u32 count = 0;
        for(u32 i = 0; i < len; ++i) {
            if(' ' == str[i] || '\t' == str[i]) {
                ++count;
            } else {
                break;
            }
        }
        CPPJSON_ASSERT(count <= 0);
#endif
        bool minus = '-' == *str;
        u32 i = ('-' == *str || '+' == *str) ? 1 : 0;
        s32 x = 0;
        if(minus) {
            for(; i < len && std::isdigit(str[i]); ++i) {
                if(mul_overflow(x, x) || sub_overflow(x, x, str[i] - '0')) {
                    return (std::numeric_limits<s32>::min)();
                }
            }
        } else {
            for(; i < len && std::isdigit(str[i]); ++i) {
                if(mul_overflow(x, x) || add_overflow(x, x, str[i] - '0')) {
                    return (std::numeric_limits<s32>::max)();
                }
            }
        }
        return x;
    }

    /**
     * @brief Convert string to signed integer
     * @param str [in] ... string
     * @param len [in] ... length of str
     * @return The converted value. If the result is overflowed, INT_MAX or INT_MIN be returned.
     */
    u32 atou(const char* str, u32 len)
    {
        CPPJSON_ASSERT(CPPJSON_NULL != str);
        CPPJSON_ASSERT(0 < len);
#ifdef _DEBUG
        u32 count = 0;
        for(u32 i = 0; i < len; ++i) {
            if(' ' == str[i] || '\t' == str[i]) {
                ++count;
            } else {
                break;
            }
        }
        CPPJSON_ASSERT(count <= 0);
#endif
        bool minus = '-' == *str;
        u32 i = ('-' == *str || '+' == *str) ? 1 : 0;
        u32 x = 0;
        for(; i < len && std::isdigit(str[i]); ++i) {
            x *= 10;
            x += str[i] - '0';
        }
        return minus ? -*reinterpret_cast<s32*>(&x) : x;
    }

    f64 atod(const char* str, u32 len)
    {
        CPPJSON_ASSERT(CPPJSON_NULL != str);
        CPPJSON_ASSERT(0 < len);
#ifdef _DEBUG
        u32 count = 0;
        for(u32 i = 0; i < len; ++i) {
            if(' ' == str[i] || '\t' == str[i]) {
                ++count;
            } else {
                break;
            }
        }
        CPPJSON_ASSERT(count <= 0);
#endif
        bool minus = '-' == *str;
        u32 i = ('-' == *str || '+' == *str) ? 1 : 0;
        f64 x = 0.0;
        for(; i < len && std::isdigit(str[i]); ++i) {
            x *= 10;
            x += str[i] - '0';
        }
        if(i < len && '.' == str[i]) {
            f64 f = 1.0;
            for(++i; i < len && std::isdigit(str[i]); ++i) {
                f *= 0.1;
                x += f * (str[i] - '0');
            }
        }
        if(i < len && ('e' == str[i] || 'E' == str[i])) {
            ++i;
            bool expminus = false;
            if(i < len) {
                switch(str[i]) {
                case '+':
                    ++i;
                    break;
                case '-':
                    expminus = true;
                    ++i;
                    break;
                }
            }
            s64 e = 0;
            for(; i < len && std::isdigit(str[i]); ++i) {
                e = e * 10 + (str[i] - '0');
            }
            x *= std::pow(10.0, static_cast<f64>(expminus ? -e : e));
        }
        return minus ? -x : x;
    }

    //#endif
} // namespace

//--- JsonStringProxy
JsonStringProxy::operator bool() const
{
    return Invalid != element_ && JsonType::String == (*parent_)[element_].type_;
}

s32 JsonStringProxy::compare(u32 length, const char* str) const
{
    CPPJSON_ASSERT(CPPJSON_NULL != str);
    if(length != this->length()){
        return -1;
    }
    return ::strncmp(str, this->str(), length);
}

u32 JsonStringProxy::length() const
{
    if(Invalid != element_) {
        const JsonValue& value = (*parent_)[element_];
        CPPJSON_ASSERT(JsonType::String == value.type_);
        return value.length_;
    }
    return 0;
}

const char* JsonStringProxy::str() const
{
    if(Invalid != element_) {
        const JsonValue& value = (*parent_)[element_];
        CPPJSON_ASSERT(JsonType::String == value.type_);
        return parent_->row_data(value.position_);
    } else {
        return JsonStrEmpty;
    }
}

//--- JsonValueProxy
JsonValueProxy::operator bool() const
{
    return Invalid != element_
           && JsonType::Object != (*parent_)[element_].type_
           && JsonType::Array != (*parent_)[element_].type_
           && JsonType::KeyValue != (*parent_)[element_].type_;
}

JsonType JsonValueProxy::type() const
{
    return (*parent_)[element_].type_;
}

s32 JsonValueProxy::asInt() const
{
    CPPJSON_ASSERT(JsonType::Number == type() || JsonType::Integer == type());
    const JsonValue& value = (*parent_)[element_];
    u32 length = value.length_;
    const char* str = parent_->row_data(value.position_);
    return atoi(str, length);
}

s32 JsonValueProxy::asInt(s32 defaultValue) const
{
    if(Invalid == element_) {
        return defaultValue;
    }
    return asInt();
}

u32 JsonValueProxy::asUInt() const
{
    CPPJSON_ASSERT(JsonType::Number == type() || JsonType::Integer == type());
    const JsonValue& value = (*parent_)[element_];
    u32 length = value.length_;
    const char* str = parent_->row_data(value.position_);
    return atou(str, length);
}

f32 JsonValueProxy::asFloat() const
{
    CPPJSON_ASSERT(JsonType::Number == type() || JsonType::Integer == type());
    const JsonValue& value = (*parent_)[element_];
    u32 length = value.length_;
    const char* str = parent_->row_data(value.position_);
    return static_cast<f32>(atod(str, length));
}

f64 JsonValueProxy::asDouble() const
{
    CPPJSON_ASSERT(JsonType::Number == type() || JsonType::Integer == type());
    const JsonValue& value = (*parent_)[element_];
    u32 length = value.length_;
    const char* str = parent_->row_data(value.position_);
    return atod(str, length);
}

JsonStringProxy JsonValueProxy::asString() const
{
    if(Invalid != element_) {
        CPPJSON_ASSERT(JsonType::String == type());
        return {parent_, element_};
    }
    return {CPPJSON_NULL, Invalid};
}

bool JsonValueProxy::asBool() const
{
    CPPJSON_ASSERT(JsonType::Boolean == type());
    const JsonValue& value = (*parent_)[element_];
    const char* str = parent_->row_data(value.position_);
    return 't' == str[0];
}

JsonObjectProxy JsonValueProxy::asObject() const
{
    if(Invalid != element_) {
        CPPJSON_ASSERT(JsonType::Object == type());
        return {parent_, element_};
    }
    return {CPPJSON_NULL, Invalid};
}

JsonArrayProxy JsonValueProxy::asArray() const
{
    if(Invalid != element_) {
        CPPJSON_ASSERT(JsonType::Array == type());
        return {parent_, element_};
    }
    return {CPPJSON_NULL, Invalid};
}

//--- JsonKeyVlaueProxy
JsonKeyVlaueProxy::operator bool() const
{
    return Invalid != element_ && JsonType::KeyValue == (*parent_)[element_].type_;
}

JsonStringProxy JsonKeyVlaueProxy::key() const
{
    CPPJSON_ASSERT(*this);
    CPPJSON_ASSERT(JsonType::KeyValue == (*parent_)[element_].type_);
    CPPJSON_ASSERT(JsonType::String == (*parent_)[(*parent_)[element_].key_value_.key_].type_);
    return {parent_, (*parent_)[element_].key_value_.key_};
}

JsonValueProxy JsonKeyVlaueProxy::value() const
{
    CPPJSON_ASSERT(*this);
    CPPJSON_ASSERT(JsonType::KeyValue == (*parent_)[element_].type_);
    return {parent_, (*parent_)[element_].key_value_.value_};
}

//--- JsonObjectProxy
JsonObjectProxy::operator bool() const
{
    return Invalid != element_ && JsonType::Object == (*parent_)[element_].type_;
}

u32 JsonObjectProxy::size() const
{
    CPPJSON_ASSERT(Invalid != element_);
    return (*parent_)[element_].object_.size_;
}

u32 JsonObjectProxy::existsKey(u32 length, const char* key) const
{
    CPPJSON_ASSERT(Invalid != element_);
    const JsonValue& object = (*parent_)[element_];
    JsonStringProxy keyString = {parent_, Invalid};
    u32 child = object.object_.head_;
    while(Invalid != child) {
        const JsonValue& keyValue = (*parent_)[child];
        keyString.element_ = keyValue.key_value_.key_;
        if(0 == keyString.compare(length, key)) {
            return child;
        }
        child = (*parent_)[child].next_;
    }
    return Invalid;
}

u32 JsonObjectProxy::existsKey(u32 length, const char* key, JsonType type) const
{
    CPPJSON_ASSERT(Invalid != element_);
    u32 index = existsKey(length, key);
    if(Invalid == index) {
        return Invalid;
    }
    u32 value_index = (*parent_)[index].key_value_.value_;
    CPPJSON_ASSERT(Invalid != value_index);
    return type == (*parent_)[value_index].type_ ? value_index : Invalid;
}

bool JsonObjectProxy::tryGetValue(JsonValueProxy& value, u32 length, const char* key) const
{
    u32 index = existsKey(length, key);
    if(Invalid != index) {
        value = {parent_, (*parent_)[index].key_value_.value_};
        return true;
    }
    return false;
}

bool JsonObjectProxy::tryGetValue(JsonValueProxy& value, JsonType type, u32 length, const char* key) const
{
    u32 index = existsKey(length, key);
    if(Invalid == index) {
        return false;
    }
    index = (*parent_)[index].key_value_.value_;
    if(type == (*parent_)[index].type_) {
        value = {parent_, index};
        return true;
    } else {
        return false;
    }
}

JsonValueProxy JsonObjectProxy::getValue(u32 length, const char* key) const
{
    u32 index = existsKey(length, key);
    if(Invalid != index) {
        return {parent_, (*parent_)[index].key_value_.value_};
    }
    return {CPPJSON_NULL, Invalid};
}

JsonStringProxy JsonObjectProxy::getAsString(u32 length, const char* key) const
{
    CPPJSON_ASSERT(CPPJSON_NULL != key);
    u32 index = existsKey(length, key);
    if(Invalid != index) {
        return {parent_, (*parent_)[index].key_value_.value_};
    } else {
        return {CPPJSON_NULL, Invalid};
    }
}

template<>
s32 JsonObjectProxy::getAs<s32>(u32 length, const char* key, s32 defaultValue) const
{
    CPPJSON_ASSERT(CPPJSON_NULL != key);
    u32 index = existsKey(length, key);
    if(Invalid == index) {
        return defaultValue;
    }
    index = (*parent_)[index].key_value_.value_;
    if(Invalid == index) {
        return defaultValue;
    }
    if(JsonType::Integer != (*parent_)[index].type_) {
        return defaultValue;
    }
    JsonValueProxy intValue = {parent_, index};
    return intValue.asInt();
}

template<>
f32 JsonObjectProxy::getAs<f32>(u32 length, const char* key, f32 defaultValue) const
{
    CPPJSON_ASSERT(CPPJSON_NULL != key);
    u32 index = existsKey(length, key);
    if(Invalid == index) {
        return defaultValue;
    }
    index = (*parent_)[index].key_value_.value_;
    if(Invalid == index) {
        return defaultValue;
    }
    JsonType type = (*parent_)[index].type_;
    if(JsonType::Integer != type || JsonType::Number != type) {
        return defaultValue;
    }
    JsonValueProxy value = {parent_, index};
    return JsonType::Integer == type ? value.asInt() : value.asFloat();
}

template<>
bool JsonObjectProxy::getAs<bool>(u32 length, const char* key, bool defaultValue) const
{
    CPPJSON_ASSERT(CPPJSON_NULL != key);
    u32 index = existsKey(length, key);
    if(Invalid == index) {
        return defaultValue;
    }
    index = (*parent_)[index].key_value_.value_;
    if(Invalid == index) {
        return defaultValue;
    }
    JsonType type = (*parent_)[index].type_;
    if(JsonType::Boolean != type) {
        return defaultValue;
    }
    JsonValueProxy value = {parent_, index};
    return value.asBool();
}

namespace
{
    bool getFloatArray(u32 size, f32* values, const JsonObjectProxy& object, u32 length, const char* key)
    {
        u32 index = object.existsKey(length, key);
        if(Invalid == index) {
            return false;
        }
        index = (*object.parent_)[index].key_value_.value_;
        if(Invalid == index) {
            return false;
        }

        JsonType type = (*object.parent_)[index].type_;
        if(JsonType::Array != type || size != (*object.parent_)[index].length_) {
            return false;
        }
        u32 count = 0;
        u32 child = (*object.parent_)[index].array_.head_;
        while(Invalid != child) {
            JsonValueProxy value = {object.parent_, child};
            if(JsonType::Number != value.type() && JsonType::Integer != value.type()) {
                return false;
            }
            values[count] = value.asFloat();
            ++count;
            if(size <= count) {
                break;
            }
        }
        return true;
    }
} // namespace

//--- JsonArrayIterator
bool JsonArrayIterator::next()
{
    current_ = element_;
    if(Invalid != element_){
        element_ = (*parent_)[element_].next_;
    }
    return Invalid != current_;
}

JsonValueProxy JsonArrayIterator::current() const
{
    return {parent_, current_};
}

//--- JsonArrayProxy
JsonArrayProxy::operator bool() const
{
    return Invalid != element_ && JsonType::Array == (*parent_)[element_].type_;
}

bool JsonArrayProxy::isHomogeneous(JsonType type) const
{
    if(Invalid == element_) {
        return true;
    }
    u32 child = (*parent_)[element_].array_.head_;
    while(Invalid != child) {
        if(type != (*parent_)[child].type_) {
            return false;
        }
        child = (*parent_)[child].next_;
    }
    return true;
}

u32 JsonArrayProxy::size() const
{
    return (Invalid != element_) ? (*parent_)[element_].array_.size_ : 0;
}

JsonValueProxy JsonArrayProxy::operator[](u32 index) const
{
    CPPJSON_ASSERT(index < size());
    CPPJSON_ASSERT(Invalid != element_);
    u32 count = 0;
    u32 child = (*parent_)[element_].array_.head_;
    while(Invalid != child) {
        if(count == index) {
            return {parent_, child};
        }
        child = (*parent_)[child].next_;
        ++count;
    }
    return {CPPJSON_NULL, Invalid};
}

JsonArrayIterator JsonArrayProxy::begin() const
{
    CPPJSON_ASSERT(Invalid != element_);
    return {parent_, (*parent_)[element_].array_.head_, Invalid};
}

//--- Buffer
JsonParser::Buffer::Buffer(cppjson_malloc allocator, cppjson_free deallocator)
    : allocator_(allocator)
    , deallocator_(deallocator)
    , capacity_(0)
    , size_(0)
    , values_(CPPJSON_NULL)
{
    CPPJSON_ASSERT((CPPJSON_NULL != allocator && CPPJSON_NULL != deallocator) || (CPPJSON_NULL == allocator && CPPJSON_NULL == deallocator));
    if(CPPJSON_NULL == allocator_) {
        allocator_ = ::malloc;
    }
    if(CPPJSON_NULL == deallocator_) {
        deallocator_ = ::free;
    }
}

JsonParser::Buffer::~Buffer()
{
    deallocator_(values_);
}

u32 JsonParser::Buffer::capacity() const
{
    return capacity_;
}

u32 JsonParser::Buffer::size() const
{
    return size_;
}

void JsonParser::Buffer::clear()
{
    size_ = 0;
}

void JsonParser::Buffer::push_back(const JsonValue& value)
{
    if(capacity_ <= size_) {
        u32 capacity = capacity_ + ExpandSize;
        JsonValue* values = reinterpret_cast<JsonValue*>(allocator_(sizeof(JsonValue) * capacity));
        if(CPPJSON_NULL != values_) {
            ::memcpy(values, values_, sizeof(JsonValue) * capacity_);
        }
        deallocator_(values_);
        capacity_ = capacity;
        values_ = values;
    }
    values_[size_] = value;
    ++size_;
}

const JsonValue& JsonParser::Buffer::operator[](u32 index) const
{
    CPPJSON_ASSERT(index < size_);
    return values_[index];
}

JsonValue& JsonParser::Buffer::operator[](u32 index)
{
    CPPJSON_ASSERT(index < size_);
    return values_[index];
}

//--- JsonParser
JsonParser::JsonParser(cppjson_malloc allocator, cppjson_free deallocator, u32 max_nests)
    : allocator_(allocator)
    , deallocator_(deallocator)
    , max_nests_(max_nests)
    , nest_count_(0)
    , begin_(CPPJSON_NULL)
    , current_(CPPJSON_NULL)
    , end_(CPPJSON_NULL)
    , buffer_(allocator, deallocator)
{
    CPPJSON_ASSERT((CPPJSON_NULL != allocator && CPPJSON_NULL != deallocator) || (CPPJSON_NULL == allocator && CPPJSON_NULL == deallocator));
    if(CPPJSON_NULL == allocator_) {
        allocator_ = ::malloc;
    }
    if(CPPJSON_NULL == deallocator_) {
        deallocator_ = ::free;
    }
}

JsonParser::~JsonParser()
{
}

bool JsonParser::parse(cursor head, cursor end)
{
    CPPJSON_ASSERT(head <= end);
    nest_count_ = 0;
    begin_ = current_ = head;
    end_ = end;
    buffer_.clear();
    skip_bom();
    skip_space();
    JsonResult result = parse_value();
    skip_space();
    return result.success_ && end_ <= current_;
}

u32 JsonParser::size() const
{
    return buffer_.size();
}

JsonValueProxy JsonParser::getRoot() const
{
    CPPJSON_ASSERT(0 < size());
    return {this, 0};
}

const JsonValue& JsonParser::operator[](u32 index) const
{
    CPPJSON_ASSERT(index < size());
    return buffer_[index];
}

JsonParser::cursor JsonParser::row_data(u32 index) const
{
    CPPJSON_ASSERT(index < std::distance(begin_, end_));
    return begin_ + index;
}

void JsonParser::add_element_to_object(u32 object_index, u32 element_index)
{
    CPPJSON_ASSERT(object_index < buffer_.size());
    CPPJSON_ASSERT(JsonType::Object == buffer_[object_index].type_);
    CPPJSON_ASSERT(Invalid == buffer_[element_index].next_);
    if(Invalid == buffer_[object_index].object_.head_) {
        buffer_[object_index].object_.head_ = element_index;
    } else {
        u32 prev_index = buffer_[object_index].object_.head_;
        while(Invalid != buffer_[prev_index].next_) {
            prev_index = buffer_[prev_index].next_;
        }
        CPPJSON_ASSERT(element_index != prev_index);
        buffer_[prev_index].next_ = element_index;
    }
    buffer_[object_index].object_.size_ += 1;
}

void JsonParser::add_element_to_array(u32 array_index, u32 element_index)
{
    CPPJSON_ASSERT(array_index < buffer_.size());
    CPPJSON_ASSERT(JsonType::Array == buffer_[array_index].type_);
    CPPJSON_ASSERT(Invalid == buffer_[element_index].next_);
    if(Invalid == buffer_[array_index].array_.head_) {
        buffer_[array_index].array_.head_ = element_index;
    } else {
        u32 prev_index = buffer_[array_index].array_.head_;
        while(Invalid != buffer_[prev_index].next_) {
            prev_index = buffer_[prev_index].next_;
        }
        CPPJSON_ASSERT(element_index != prev_index);
        buffer_[prev_index].next_ = element_index;
    }
    buffer_[array_index].array_.size_ += 1;
}

void JsonParser::skip_bom()
{
    size_t size = std::distance(current_, end_);
    if(3 <= size) {
        u8 c0 = *reinterpret_cast<const u8*>(&current_[0]);
        u8 c1 = *reinterpret_cast<const u8*>(&current_[1]);
        u8 c2 = *reinterpret_cast<const u8*>(&current_[2]);
        if(0xEFU == c0 && 0xBBU == c1 && 0xBFU == c2) {
            current_ += 3;
        }
    }
}

void JsonParser::skip_space()
{
    while(current_ < end_) {
        switch(*current_) {
        case 0x09:
        case 0x0A:
        case 0x0D:
        case 0x20:
            ++current_;
            break;
        default:
            return;
        }
    }
}

JsonResult JsonParser::parse_value()
{
    if(end_ <= current_) {
        return {false, 0};
    }
    switch(*current_) {
    case '"':
        return parse_string();
    case '{':
        return parse_object();
    case '[':
        return parse_array();
    case 't':
        return parse_true();
    case 'f':
        return parse_false();
    case 'n':
        return parse_null();
    default:
        if('-' == *current_ || ('0' <= *current_ && *current_ <= '9')) {
            return parse_number();
        }
        break;
    }
    return {false, 0};
}

JsonResult JsonParser::parse_object()
{
    CPPJSON_ASSERT(current_ < end_ && '{' == *current_);
    ++current_;
    if(end_ <= current_ || max_nests_ <= nest_count_) {
        return {false, 0};
    }
    ++nest_count_;
    u32 object_index = buffer_.size();
    {
        JsonValue value;
        value.position_ = get_position();
        value.length_ = 0;
        value.next_ = Invalid;
        value.type_ = JsonType::Object;
        value.object_.size_ = 0;
        value.object_.head_ = Invalid;
        buffer_.push_back(value);
    }

    bool prev_begin = true;
    bool prev_comma = false;
    while(current_ < end_) {
        skip_space();
        u32 position = get_position();

        switch(*current_) {
        case '}': {
            if(prev_comma) {
                return {false, 0};
            }
            buffer_[object_index].length_ = position - buffer_[object_index].position_ - 1;
            ++current_;
            --nest_count_;
            return {true, object_index};
        }
        case '"': {
            prev_begin = false;
            prev_comma = false;
            JsonResult key_result = parse_string();
            if(!key_result.success_) {
                return key_result;
            }
            skip_space();
            if(end_ <= current_ || ':' != *current_) {
                return {false, 0};
            }
            ++current_;
            skip_space();
            JsonResult value_result = parse_value();
            if(!value_result.success_) {
                return value_result;
            }
            JsonValue key_value;
            key_value.position_ = position;
            key_value.length_ = buffer_[value_result.index_].position_ + buffer_[value_result.index_].length_ - position;
            key_value.next_ = Invalid;
            key_value.type_ = JsonType::KeyValue;
            key_value.key_value_.key_ = key_result.index_;
            key_value.key_value_.value_ = value_result.index_;
            u32 key_value_index = buffer_.size();
            buffer_.push_back(key_value);
            add_element_to_object(object_index, key_value_index);
            break;
        }
        case ',':
            if(prev_begin || prev_comma) {
                return {false, 0};
            }
            prev_begin = false;
            prev_comma = true;
            ++current_;
            break;
        default:
            return {false, 0};
        }
    }
    return {false, 0};
}

JsonResult JsonParser::parse_array()
{
    CPPJSON_ASSERT(current_ < end_ && '[' == *current_);
    ++current_;
    if(end_ <= current_ || max_nests_ <= nest_count_) {
        return {false, 0};
    }
    ++nest_count_;
    u32 array_index = buffer_.size();
    {
        JsonValue value;
        value.position_ = get_position();
        value.length_ = 0;
        value.next_ = Invalid;
        value.type_ = JsonType::Array;
        value.array_.size_ = 0;
        value.array_.head_ = Invalid;
        buffer_.push_back(value);
    }

    bool prev_begin = true;
    bool prev_comma = false;
    while(current_ < end_) {
        skip_space();
        u32 position = get_position();

        switch(*current_) {
        case ']': {
            if(prev_comma) {
                return {false, 0};
            }
            u32 length = position - buffer_[array_index].position_;
            buffer_[array_index].length_ = length;
            ++current_;
            --nest_count_;
            return {true, array_index};
        }
        case ',':
            if(prev_begin || prev_comma) {
                return {false, 0};
            }
            prev_begin = false;
            prev_comma = true;
            ++current_;
            break;
        default: {
            if(!prev_begin && !prev_comma) {
                return {false, 0};
            }
            JsonResult result = parse_value();
            if(!result.success_) {
                return result;
            }
            add_element_to_array(array_index, result.index_);
            prev_begin = false;
            prev_comma = false;
            break;
        }
        }
    }
    return {false, 0};
}

JsonResult JsonParser::parse_string()
{
    CPPJSON_ASSERT(current_ < end_ && '"' == *current_);
    ++current_;
    if(end_ <= current_) {
        return {false, 0};
    }
    u32 position = get_position();
    while(current_ < end_) {
        switch(*current_) {
        case '\\': {
            s32 result = parse_escape();
            if(result <= 0) {
                return {false, 0};
            }
            current_ += result;
            break;
        }
        case '"': {
            u32 size = get_position() - position;
            u32 index = buffer_.size();
            JsonValue value;
            value.position_ = position;
            value.length_ = size;
            value.next_ = Invalid;
            value.type_ = JsonType::String;
            buffer_.push_back(value);
            ++current_;
            return {true, index};
        }
        default: {
            s32 result = parse_utf8();
            if(result <= 0) {
                return {false, 0};
            }
            current_ += result;
            break;
        }
        }
    }
    return {false, 0};
}

JsonResult JsonParser::parse_true()
{
    CPPJSON_ASSERT(current_ < end_ && 't' == *current_);
    if(end_ <= (current_ + 3)) {
        return {false, 0};
    }
    if('r' == current_[1]
       && 'u' == current_[2]
       && 'e' == current_[3]) {
        u32 index = buffer_.size();
        JsonValue value;
        value.position_ = get_position();
        value.length_ = 4;
        value.next_ = Invalid;
        value.type_ = JsonType::Boolean;
        buffer_.push_back(value);
        current_ += 4;
        return {true, index};
    }
    return {false, 0};
}

JsonResult JsonParser::parse_false()
{
    CPPJSON_ASSERT(current_ < end_ && 'f' == *current_);
    if(end_ <= (current_ + 4)) {
        return {false, 0};
    }
    if('a' == current_[1]
       && 'l' == current_[2]
       && 's' == current_[3]
       && 'e' == current_[4]) {
        u32 index = buffer_.size();
        JsonValue value;
        value.position_ = get_position();
        value.length_ = 5;
        value.next_ = Invalid;
        value.type_ = JsonType::Boolean;
        buffer_.push_back(value);
        current_ += 5;
        return {true, index};
    }
    return {false, 0};
}

JsonResult JsonParser::parse_null()
{
    CPPJSON_ASSERT(current_ < end_ && 'n' == *current_);
    if(end_ <= (current_ + 3)) {
        return {false, 0};
    }
    if('u' == current_[1]
       && 'l' == current_[2]
       && 'l' == current_[3]) {
        u32 index = buffer_.size();
        JsonValue value;
        value.position_ = get_position();
        value.length_ = 4;
        value.next_ = Invalid;
        value.type_ = JsonType::Null;
        buffer_.push_back(value);
        current_ += 4;
        return {true, index};
    }
    return {false, 0};
}

JsonResult JsonParser::parse_number()
{
    CPPJSON_ASSERT(current_ < end_);
    CPPJSON_ASSERT('-' == *current_ || ('0' <= *current_ && *current_ <= '9'));
    bool minus = '-' == *current_;
    bool zero = '0' == *current_;
    u32 position = get_position();
    ++current_;

    JsonType type = JsonType::Integer;
    if(minus) {
        if(end_ <= current_) {
            return {false, 0};
        }
        if(!('0' <= *current_ && *current_ <= '9')) {
            return {false, 0};
        }
        zero = '0' == *current_;
        ++current_;
    }
    if(end_ <= current_) {
        return get_number(position, type);
    }
    if(!zero) {
        parse_integer();
        if(end_ <= current_) {
            return get_number(position, type);
        }
    }

    if('.' == *current_) {
        type = JsonType::Number;
        if(parse_fraction() < 0) {
            return {false, 0};
        }
        if(end_ <= current_) {
            return get_number(position, type);
        }
    }
    if('E' == *current_ || 'e' == *current_) {
        type = JsonType::Number;
        if(parse_exponent() < 0) {
            return {false, 0};
        }
    }
    return get_number(position, type);
}

s32 JsonParser::parse_escape()
{
    CPPJSON_ASSERT(current_ < end_ && '\\' == *current_);
    ++current_;
    if(end_ <= current_) {
        return false;
    }
    switch(*current_) {
    case '"':
    case '\\':
    case '/':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
        return 1;
    case 'u':
        return parse_4hexdigit();
    default:
        return -1;
    }
}

s32 JsonParser::parse_utf8()
{
    u8 u0 = *((u8*)current_);
    if(u0 <= 0x7FU) {
        return 0x20U <= u0 ? 1 : -1;
    }
    if(0xC2U <= u0 && u0 < 0xE0U) {
        if(end_ <= (current_ + 1)) {
            return -1;
        }
        return 2;
    }

    if(0xE0U <= u0 && u0 < 0xF0U) {
        if(end_ <= (current_ + 2)) {
            return -1;
        }
        return 3;
    }

    if(0xF0U <= u0 && u0 < 0xF8U) {
        if(end_ <= (current_ + 3)) {
            return -1;
        }
        return 4;
    }
    return -1;
}

s32 JsonParser::parse_4hexdigit()
{
    CPPJSON_ASSERT(current_ < end_ && 'u' == *current_);
    ++current_;
    if(end_ <= current_) {
        return -1;
    }
    if(!is_hex_digit(*current_)) {
        return -1;
    }

    if(end_ <= (current_ + 1)) {
        return -1;
    }
    if(!is_hex_digit(*(current_ + 1))) {
        return -1;
    }

    if(end_ <= (current_ + 2)) {
        return -1;
    }
    if(!is_hex_digit(*(current_ + 2))) {
        return -1;
    }

    if(end_ <= (current_ + 3)) {
        return -1;
    }
    if(!is_hex_digit(*(current_ + 3))) {
        return -1;
    }
    return 4;
}

void JsonParser::parse_integer()
{
    while(current_ < end_) {
        if('0' <= *current_ && *current_ <= '9') {
            ++current_;
        } else {
            return;
        }
    }
}

s32 JsonParser::parse_fraction()
{
    CPPJSON_ASSERT(current_ < end_ && '.' == *current_);
    ++current_;
    if(end_ <= current_) {
        return -1;
    }
    if(!('0' <= *current_ && *current_ <= '9')) {
        return -1;
    }

    s32 count = 0;
    while(current_ < end_) {
        if(!('0' <= *current_ && *current_ <= '9')) {
            break;
        }
        ++current_;
        ++count;
    }
    return count;
}

s32 JsonParser::parse_exponent()
{
    CPPJSON_ASSERT(current_ < end_ && ('E' == *current_ || 'e' == *current_));
    ++current_;
    if(end_ <= current_) {
        return -1;
    }
    if('-' == *current_ || '+' == *current_) {
        ++current_;
        if(end_ <= current_) {
            return -1;
        }
    }

    if(!('0' <= *current_ && *current_ <= '9')) {
        return -1;
    }
    s32 count = 0;
    while(current_ < end_) {
        if(!('0' <= *current_ && *current_ <= '9')) {
            break;
        }
        ++current_;
        ++count;
    }
    return count;
}

u32 JsonParser::get_position() const
{
    u64 duration = std::distance(begin_, current_);
    CPPJSON_ASSERT(duration <= 0xFFFFFFFFU);
    return static_cast<u32>(duration);
}

JsonResult JsonParser::get_number(u32 begin, JsonType type)
{
    u32 index = buffer_.size();
    JsonValue value;
    value.position_ = begin;
    value.length_ = get_position() - begin;
    value.next_ = Invalid;
    value.type_ = type;
    buffer_.push_back(value);
    return {true, index};
}

bool JsonParser::is_hex_digit(s32 c)
{
    if('0' <= c && c <= '9') {
        return true;
    }
    if('A' <= c && c <= 'F') {
        return true;
    }
    if('a' <= c && c <= 'f') {
        return true;
    }
    return false;
}
} // namespace cppjson

