#ifndef INC_CPPJSON_H_
#define INC_CPPJSON_H_
/*
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
#include <cstdint>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace cppjson
{
#ifndef CPPJSON_TYPES
#define CPPJSON_TYPES
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

    typedef char Char;
    typedef bool boolean;
#ifdef _MSC_VER
    typedef s32 off_t;
    #ifndef CPPJSON_FSEEK
    #define CPPJSON_FSEEK(f,p,o) fseek((f),(p),(o))
    #endif

    #ifndef CPPJSON_FTELL
    #define CPPJSON_FTELL(f) ftell((f))
    #endif

#else
    typedef s32 off_t;
    #ifndef CPPJSON_FSEEK
    #define CPPJSON_FSEEK(f,p,o) fseek((f),(p),(o))
    #endif

    #ifndef CPPJSON_FTELL
    #define CPPJSON_FTELL(f) ftell((f))
    #endif
#endif

#endif

#ifndef CPPJSON_MALLOC
#define CPPJSON_MALLOC(size) malloc(size)
#endif

#ifndef CPPJSON_FREE
#define CPPJSON_FREE(ptr) free(ptr)
#endif

#ifndef CPPJSON_PLACEMENT_NEW
#define CPPJSON_PLACEMENT_NEW(ptr) new(ptr)
#endif

#ifndef CPPJSON_ASSERT
#define CPPJSON_ASSERT(exp) assert(exp)
#endif


    template<class T>
    inline void swap(T& l, T& r)
    {
        T tmp(std::move(l));
        l = std::move(r);
        r = std::move(tmp);
    }

    FILE* fopen_s(const Char* filename, const Char* mode);

    //--------------------------------------------
    //---
    //--- String
    //---
    //--------------------------------------------
    class String
    {
    public:
        static const s32 ExpandSize = 8;
        static const u32 ExpandMask = ExpandSize-1;

        String();
        String(String&& rhs);
        ~String();

        void clear();
        void resize(s32 length);
        void reserve(s32 capacity);

        inline s32 capacity() const;
        inline s32 length() const;
        inline Char* c_str();
        inline const Char* c_str() const;
        inline const Char& operator[](s32 index) const;
        inline Char& operator[](s32 index);

        String& operator=(String&& rhs);
        String& operator=(const Char* str);
        inline void assign(const Char* str);
        void assign(s32 length, const Char* str);

        boolean operator==(const String& rhs) const;

        friend boolean operator==(const String& lhs, const Char* rhs);
        friend boolean operator==(const Char* lhs, const String& rhs);
    private:
        static inline s32 getCapacity(s32 capacity)
        {
            return (capacity+ExpandMask) & ~ExpandMask;
        }

        inline const Char* getBuffer() const
        {
            return (capacity_<=ExpandSize)? buffer_.small_ : buffer_.elements_;
        }
        inline Char* getBuffer()
        {
            return (capacity_<=ExpandSize)? buffer_.small_ : buffer_.elements_;
        }


        void initBuffer(s32 length);
        void createBuffer(s32 length);
        void expandBuffer(s32 length);

        s32 capacity_;
        s32 length_;

        union Buffer
        {
            Char small_[ExpandSize];
            Char* elements_;
        };
        Buffer buffer_;
    };

    inline s32 String::capacity() const
    {
        return capacity_;
    }

    inline s32 String::length() const
    {
        return length_;
    }

    inline Char* String::c_str()
    {
        return (ExpandSize<capacity_)? buffer_.elements_ : buffer_.small_;
    }

    inline const Char* String::c_str() const
    {
        return (ExpandSize<capacity_)? buffer_.elements_ : buffer_.small_;
    }

    inline const Char& String::operator[](s32 index) const
    {
        CPPJSON_ASSERT(0<=index && index<length_);
        return getBuffer()[index];
    }

    inline Char& String::operator[](s32 index)
    {
        CPPJSON_ASSERT(0<=index && index<length_);
        return getBuffer()[index];
    }

    inline void String::assign(const Char* str)
    {
        assign(static_cast<s32>(::strlen(str)), str);
    }

    inline s32 compare(const String& lhs, const String& rhs)
    {
        return ::strcmp(lhs.c_str(), rhs.c_str());
    }

    inline s32 compare(const String& lhs, const Char* rhs)
    {
        CPPJSON_ASSERT(NULL != rhs);
        return ::strcmp(lhs.c_str(), rhs);
    }

    inline s32 compare(const Char* lhs, const String& rhs)
    {
        CPPJSON_ASSERT(NULL != lhs);
        return ::strcmp(lhs, rhs.c_str());
    }

    //--------------------------------------------
    //---
    //--- Array
    //---
    //--------------------------------------------
    template<class T>
    class Array
    {
    public:
        typedef Array<T> this_type;
        typedef s32 size_type;
        typedef T* iterator;
        typedef const T* const_iterator;

        Array();
        Array(this_type&& rhs);
        explicit Array(s32 capacity);
        ~Array();

        inline s32 size() const;
        inline s32 capacity() const;

        inline T& operator[](s32 index);
        inline const T& operator[](s32 index) const;
        inline T& front();
        inline const T& front() const;
        inline T& back();
        inline const T& back() const;

        void push_back(const T& t);
        void push_back(T&& t);
        void pop_back();

        void clear();
        void reserve(s32 capacity);
        void resize(s32 size);
        void removeAt(s32 index);
        void swap(this_type& rhs);

        this_type& operator=(this_type&& rhs);
    private:
        Array(const this_type&) = delete;
        this_type& operator=(const this_type&) = delete;

        s32 capacity_;
        s32 size_;
        T *items_;
    };

    template<class T>
    Array<T>::Array()
        :capacity_(0)
        ,size_(0)
        ,items_(NULL)
    {
    }

    template<class T>
    Array<T>::Array(this_type&& rhs)
        :capacity_(rhs.capacity_)
        ,size_(rhs.size_)
        ,items_(rhs.items_)
    {
        rhs.capacity_ = 0;
        rhs.size_ = 0;
        rhs.items_ = NULL;
    }

    template<class T>
    Array<T>::Array(s32 capacity)
        :capacity_(capacity)
        ,size_(0)
        ,items_(NULL)
    {
        CPPJSON_ASSERT(0<=capacity_);
        items_ = (T*)CPPJSON_MALLOC(sizeof(T)*capacity_);
    }

    template<class T>
    Array<T>::~Array()
    {
        for(s32 i=0; i<size_; ++i){
            items_[i].~T();
        }
        CPPJSON_FREE(items_);
        items_ = NULL;
    }

    template<class T>
    inline s32 Array<T>::size() const
    {
        return size_;
    }

    template<class T>
    inline s32 Array<T>::capacity() const
    {
        return capacity_;
    }

    template<class T>
    inline T& Array<T>::operator[](s32 index)
    {
        CPPJSON_ASSERT(0<=index && index<size_);
        return items_[index];
    }

    template<class T>
    inline const T& Array<T>::operator[](s32 index) const
    {
        CPPJSON_ASSERT(0<=index && index<size_);
        return items_[index];
    }

    template<class T>
    inline T& Array<T>::front()
    {
        CPPJSON_ASSERT(0<size_);
        return items_[0];
    }

    template<class T>
    inline const T& Array<T>::front() const
    {
        CPPJSON_ASSERT(0<size_);
        return items_[0];
    }

    template<class T>
    inline T& Array<T>::back()
    {
        CPPJSON_ASSERT(0<size_);
        return items_[size_-1];
    }

    template<class T>
    inline const T& Array<T>::back() const
    {
        CPPJSON_ASSERT(0<size_);
        return items_[size_-1];
    }


    template<class T>
    void Array<T>::push_back(const T& t)
    {
        if(capacity_<=size_){
            reserve(capacity_+16);
        }
        CPPJSON_PLACEMENT_NEW(&items_[size_]) T(t);
        ++size_;
    }


    template<class T>
    void Array<T>::push_back(T&& t)
    {
        if(capacity_<=size_){
            reserve(capacity_+16);
        }
        CPPJSON_PLACEMENT_NEW(&items_[size_]) T(t);
        ++size_;
    }

    template<class T>
    void Array<T>::pop_back()
    {
        CPPJSON_ASSERT(0<size_);
        --size_;
        items_[size_].~T();
    }

    template<class T>
    void Array<T>::clear()
    {
        for(s32 i=0; i<size_; ++i){
            items_[i].~T();
        }
        size_ = 0;
    }

    template<class T>
    void Array<T>::reserve(s32 capacity)
    {
        if(capacity<=capacity_){
            return;
        }

        //allocate new buffer
        T *newItems = (T*)CPPJSON_MALLOC(capacity*sizeof(T));

        //copy construct
        for(s32 i=0; i<size_; ++i){
            CPPJSON_PLACEMENT_NEW(&newItems[i]) T(std::move(items_[i]));
            items_[i].~T();
        }
        //deallocate old buffer
        CPPJSON_FREE(items_);
        items_ = newItems;
        capacity_ = capacity;
    }

    template<class T>
    void Array<T>::resize(s32 size)
    {
        if(size < size_){
            for(s32 i=size; i<size_; ++i){
                items_[i].~T();
            }

        }else{
            reserve(size);
            for(s32 i=size_; i<size; ++i){
                CPPJSON_PLACEMENT_NEW(&items_[i]) T;
            }
        }
        size_ = size;
    }

    template<class T>
    void Array<T>::removeAt(s32 index)
    {
        CPPJSON_ASSERT(0<=index && index<size_);
        for(s32 i=index+1; i<size_; ++i){
            items_[i-1] = std::move(items_[i]);
        }
        --size_;
        items_[size_].~T();
    }

    template<class T>
    void Array<T>::swap(this_type& rhs)
    {
        swap(capacity_, rhs.capacity_);
        swap(size_, rhs.size_);
        swap(items_, rhs.items_);
    }

    template<class T>
    typename Array<T>::this_type& Array<T>::operator=(this_type&& rhs)
    {
        if(this == &rhs){
            return;
        }
        capacity_ = rhs.capacity_;
        size_ = rhs.size_;
        items_ = rhs.items_;

        rhs.capacity_ = 0;
        rhs.size_ = 0;
        rhs.items_ = NULL;
    }

    //---------------------------------------------------------------
    //---
    //--- IStream
    //---
    //---------------------------------------------------------------
    class IStream
    {
    public:
        /**
        @return Success: position, Faile: -1
        */
        virtual off_t tell() const =0;
        virtual boolean seek(off_t pos) =0;

        /**
        @return Success: input c, Fail: EOF
        */
        virtual s32 get() =0;

        /**
        @return Success: input c, Fail: EOF
        */
        virtual s32 unget(s32 c) =0;
        virtual s32 read(u8* dst, off_t size) =0;

        virtual boolean isEOF() const =0;
    protected:
        IStream(const IStream&) = delete;
        IStream& operator=(const IStream&) = delete;

        IStream()
        {}
        virtual ~IStream()
        {}
    };

    class ISStream : public IStream
    {
    public:
        ISStream();
        ISStream(ISStream&& rhs);
        ISStream(off_t length, const s8* str);
        virtual ~ISStream();

        off_t tell() const override;
        boolean seek(off_t pos) override;
        s32 get() override;
        s32 unget(s32 c) override;
        s32 read(u8* dst, off_t size) override;

        boolean isEOF() const override;

        ISStream& operator=(ISStream&& rhs);
    protected:
        ISStream(const ISStream&) = delete;
        ISStream& operator=(const ISStream&) = delete;

        off_t pos_;
        off_t length_;
        const s8* str_;
    };

    class IFStream : public IStream
    {
    public:
        IFStream();
        IFStream(IFStream&& rhs);
        IFStream(FILE* file);
        virtual ~IFStream();

        off_t tell() const override;
        boolean seek(off_t pos) override;
        s32 get() override;
        s32 unget(s32 c) override;
        s32 read(u8* dst, off_t size) override;

        boolean isEOF() const override;

        IFStream& operator=(IFStream&& rhs);
    protected:
        IFStream(const IFStream&) = delete;
        IFStream& operator=(const IFStream&) = delete;

        off_t pos_;
        FILE* file_;
    };


    //---------------------------------------------------------------
    enum JSONStatus
    {
        JSON_Error = -1,
        JSON_OK =0,
    };
    enum JSONToken
    {
        JSONToken_Error = -1,
        JSONToken_None = 0,
        JSONToken_OBJStart = '{',
        JSONToken_OBJEnd = '}',
        JSONToken_OBJColon = ':',
        JSONToken_Separator = ',',
        JSONToken_ArrayStart = '[',
        JSONToken_ArrayEnd = ']',
        JSONToken_DQuote = '"',
        JSONToken_Escape = '\\',
        JSONToken_Value = 0x80,
    };

    enum JSON
    {
        JSON_Object =1,
        JSON_Array,
        JSON_String,
        JSON_Integer,
        JSON_Float,
        JSON_True,
        JSON_False,
        JSON_Null,
    };

    //---------------------------------------------------------------
    //---
    //--- Range
    //---
    //---------------------------------------------------------------
    struct Range
    {
        off_t start_;
        off_t length_;

        inline void reset()
        {
            start_ = length_ = 0;
        }
    };

    //---------------------------------------------------------------
    //---
    //--- RangeStream
    //---
    //---------------------------------------------------------------
    class RangeStream
    {
    public:
        RangeStream(IStream* stream, const Range& range);
        off_t length() const;
        s32 read(u8* dst);
        s32 readAsString(Char* str);
        s32 readAsString(String& str);
        s32 readAsInt(s32 defaultValue=0);
        f32 readAsFloat(f32 defaultValue=0.0f);
    private:
        IStream* stream_;
        off_t start_;
        off_t length_;
    };

    //---------------------------------------------------------------
    //---
    //--- JSONEventHandler
    //---
    //---------------------------------------------------------------
    class JSONEventHandler
    {
    public:
        virtual void begin() =0;
        virtual void end() =0;
        virtual void beginObject() =0;
        virtual void endObject(RangeStream object) =0;

        virtual void beginArray() =0;
        virtual void endArray(RangeStream stream) =0;

        virtual void root(s32 type, RangeStream value) =0;
        virtual void value(s32 type, RangeStream v) =0;
        virtual void keyValue(RangeStream key, s32 type, RangeStream value) =0;

        virtual void onError(s32 line, s32 charCount) =0;
    protected:
        JSONEventHandler(const JSONEventHandler&) = delete;
        JSONEventHandler& operator=(const JSONEventHandler&) = delete;

        JSONEventHandler()
        {}
        virtual ~JSONEventHandler()
        {}
    };

    //---------------------------------------------------------------
    //---
    //--- JSONReader
    //---
    //---------------------------------------------------------------
    class JSONReader
    {
    public:
        static const s32 Flag_ErrorReported = (0x01<<0);

        JSONReader(IStream& istream, JSONEventHandler& handler);
        ~JSONReader();

        boolean read(s32 flags = 0);
    private:
        boolean skipSpace();
        boolean check(const s8* str);
        void setStart(Range& range);
        void setLength(Range& range);
        s32 onError();

        s32 getObject(Range& object);
        s32 getArray(Range& array);
        s32 getValue(Range& value);
        s32 getString(Range& str);
        /**
        @return Success:JSON_Integer of JSON_Float, Fail:JSON_Error
        */
        s32 getNumber(Range& number);
        s32 getDigit(boolean unget=true);
        boolean isNumber(s32 c) const;
        boolean isOneToNine(s32 c) const;

        s32 flags_;
        IStream& istream_;
        JSONEventHandler& handler_;
    };
}
#endif //INC_CPPJSON_H_

//#define CPPJSON_IMPLEMENTATION
#ifdef CPPJSON_IMPLEMENTATION
namespace cppjson
{

    FILE* fopen_s(const Char* filename, const Char* mode)
    {
#ifdef _MSC_VER
        FILE* file = NULL;
        return 0 == ::fopen_s(&file, filename, mode)? file : NULL;
#else
        return fopen(filename, mode);
#endif
    }

    //--------------------------------------------
    //---
    //--- String
    //---
    //--------------------------------------------
    String::String()
        :capacity_(ExpandSize)
        ,length_(0)
    {
        buffer_.small_[0] = '\0';
    }

    String::String(String&& rhs)
        :capacity_(rhs.capacity_)
        ,length_(rhs.length_)
        ,buffer_(rhs.buffer_)
    {
        rhs.capacity_ = 0;
        rhs.length_ = 0;
        rhs.buffer_.elements_ = NULL;
    }

    String::~String()
    {
        if(ExpandSize<capacity_){
            CPPJSON_FREE(buffer_.elements_);
            buffer_.elements_ = NULL;
        }
        capacity_ = 0;
        length_ = 0;
    }

    void String::clear()
    {
        length_ = 0;
        Char* buffer = getBuffer();
        buffer[0] = '\0';
    }

    void String::resize(s32 length)
    {
        CPPJSON_ASSERT(0<=length);
        reserve(length+1);
        length_ = length;
    }

    void String::reserve(s32 capacity)
    {
        CPPJSON_ASSERT(0<=capacity);
        length_ = 0;
        if(capacity<=capacity_) {
            Char* buffer = getBuffer();
            buffer[0] = '\0';
            return;
        }
        if(ExpandSize<capacity_) {
            CPPJSON_FREE(buffer_.elements_);
        }
        if(capacity<=ExpandSize){
            capacity_ = ExpandSize;
            buffer_.small_[0] = '\0';
            return;
        }

        capacity_ = getCapacity(capacity);
        buffer_.elements_ = (Char*)CPPJSON_MALLOC(capacity_);
        buffer_.elements_[0] = '\0';
    }

    String& String::operator=(String&& rhs)
    {
        if(this == &rhs){
            return *this;
        }
        capacity_ = rhs.capacity_;
        length_ = rhs.length_;
        buffer_ = rhs.buffer_;

        rhs.capacity_ = 0;
        rhs.length_ = 0;
        rhs.buffer_.elements_ = NULL;
        return *this;
    }

    String& String::operator=(const Char* str)
    {
        CPPJSON_ASSERT(NULL != str);
        s32 length = static_cast<s32>(::strlen(str));
        assign(length, str);
        return *this;
    }

    void String::assign(s32 length, const Char* str)
    {
        CPPJSON_ASSERT(NULL != str);
        CPPJSON_ASSERT(static_cast<size_t>(length)<=::strlen(str));
        if(length<=0){
            clear();
            return;
        }
        createBuffer(length);
        length_ = length;
        Char* buffer = getBuffer();
        ::memcpy(buffer, str, sizeof(Char)*length_);
        buffer[length_] = '\0';
    }

    void String::initBuffer(s32 length)
    {
        if(length<ExpandSize){
            capacity_ = ExpandSize;
            length_ = length;
            buffer_.small_[length_] = '\0';
            return;
        }
        s32 capacity = length+1;
        capacity_ = getCapacity(capacity);
        length_ = length;
        buffer_.elements_ = (Char*)CPPJSON_MALLOC(capacity_);
        buffer_.elements_[length_] = '\0';
    }

    void String::createBuffer(s32 length)
    {
        s32 capacity = length+1;
        if(capacity<=capacity_){
            return;
        }
        if(capacity<=ExpandSize){
            capacity_ = ExpandSize;
            return;
        }
        if(ExpandSize<capacity_){
            CPPJSON_FREE(buffer_.elements_);
        }

        capacity_ = getCapacity(capacity);
        buffer_.elements_ = (Char*)CPPJSON_MALLOC(capacity_);
    }

    void String::expandBuffer(s32 length)
    {
        s32 capacity = length+1;
        if(capacity<=capacity_) {
            return;
        }
        if(capacity<=ExpandSize){
            capacity_ = ExpandSize;
            return;
        }
        capacity = getCapacity(capacity);
        Char* elements = (Char*)CPPJSON_MALLOC(capacity);
        Char* buffer = getBuffer();
        ::memcpy(elements, buffer, sizeof(Char)*capacity_);
        if(ExpandSize<capacity_){
            CPPJSON_FREE(buffer_.elements_);
        }

        capacity_ = capacity;
        buffer_.elements_ = elements;
    }

    boolean String::operator==(const String& rhs) const
    {
        return (length() == rhs.length())? 0 == ::strncmp(c_str(), rhs.c_str(), length()) : false;
    }

    boolean operator==(const String& lhs, const Char* rhs)
    {
        s32 len= static_cast<s32>(::strlen(rhs));
        return (lhs.length() == len)? 0 == ::strncmp(lhs.c_str(), rhs, len) : false;
    }

    boolean operator==(const Char* lhs, const String& rhs)
    {
        s32 len= static_cast<s32>(::strlen(lhs));
        return (len == rhs.length())? 0 == ::strncmp(lhs, rhs.c_str(), len) : false;
    }

    //---------------------------------------------------------------
    //---
    //--- IStream
    //---
    //---------------------------------------------------------------
    ISStream::ISStream()
        :pos_(0)
        ,length_(0)
        ,str_(NULL)
    {
    }

    ISStream::ISStream(ISStream&& rhs)
        :pos_(rhs.pos_)
        ,length_(rhs.length_)
        ,str_(rhs.str_)
    {
        rhs.pos_ = 0;
        rhs.length_ = 0;
        rhs.str_ = NULL;
    }

    ISStream::ISStream(off_t length, const s8* str)
        :pos_(0)
        ,length_(length)
        ,str_(str)
    {
        CPPJSON_ASSERT(0<=length_);
        CPPJSON_ASSERT(NULL != str_);
    }

    ISStream::~ISStream()
    {
        pos_ = 0;
        length_ = 0;
        str_ = NULL;
    }

    off_t ISStream::tell() const
    {
        return pos_;
    }

    boolean ISStream::seek(off_t pos)
    {
        CPPJSON_ASSERT(0<=pos && pos<length_);
        pos_ = pos;
        return true;
    }

    s32 ISStream::get()
    {
        s32 ret=-1;
        if(pos_<length_){
            ret = str_[pos_];
            ++pos_;
        }
        return ret;
    }

    s32 ISStream::unget(s32 c)
    {
        --pos_;
        CPPJSON_ASSERT(0<=pos_ && pos_<length_);
        CPPJSON_ASSERT(str_[pos_] == c);
        return c;
    }

    s32 ISStream::read(u8* dst, off_t size)
    {
        CPPJSON_ASSERT(NULL != dst);
        off_t end = pos_+size;
        if(length_<end){
            return 0;
        }
        for(off_t i=0; i<size; ++i){
            dst[i] = str_[pos_];
            ++pos_;
        }
        return 1;
    }

    boolean ISStream::isEOF() const
    {
        return length_<=pos_;
    }

    ISStream& ISStream::operator=(ISStream&& rhs)
    {
        if(this == &rhs){
            return *this;
        }
        pos_ = rhs.pos_;
        length_ = rhs.length_;
        str_ = rhs.str_;

        rhs.pos_ = 0;
        rhs.length_ = 0;
        rhs.str_ = NULL;
        return *this;
    }

    //---------------------------------------------------------------
    //---
    //--- IFStream
    //---
    //---------------------------------------------------------------
    IFStream::IFStream()
        :pos_(0)
        ,file_(NULL)
    {
    }

    IFStream::IFStream(IFStream&& rhs)
        :pos_(rhs.pos_)
        ,file_(rhs.file_)
    {
        rhs.pos_ = 0;
        rhs.file_ = NULL;
    }

    IFStream::IFStream(FILE* file)
        :pos_(0)
        ,file_(file)
    {
        CPPJSON_ASSERT(NULL != file_);
#if _MSC_VER
        pos_ = CPPJSON_FTELL(file_);
#else
        pos_ = CPPJSON_FTELL(file_);
#endif
    }

    IFStream::~IFStream()
    {
        pos_ = 0;
        file_ = NULL;
    }

    off_t IFStream::tell() const
    {
#if _MSC_VER
        return CPPJSON_FTELL(file_)-pos_;
#else
        return CPPJSON_FTELL(file_)-pos_;
#endif
    }

    boolean IFStream::seek(off_t pos)
    {
#if _MSC_VER
        return 0<=CPPJSON_FSEEK(file_, pos_+pos, SEEK_SET);
#else 
        return 0<=CPPJSON_FSEEK(file_, pos_+pos, SEEK_SET);
#endif
    }

    s32 IFStream::get()
    {
        return fgetc(file_);
    }

    s32 IFStream::unget(s32 c)
    {
        return ungetc(c, file_);
    }

    s32 IFStream::read(u8* dst, off_t size)
    {
        CPPJSON_ASSERT(NULL != dst);
        return static_cast<s32>(fread(dst, size, 1, file_));
    }

    boolean IFStream::isEOF() const
    {
        return 0!=feof(file_);
    }

    IFStream& IFStream::operator=(IFStream&& rhs)
    {
        if(this == &rhs){
            return *this;
        }
        pos_ = rhs.pos_;
        file_ = rhs.file_;

        rhs.pos_ = 0;
        rhs.file_ = NULL;
        return *this;
    }

    //---------------------------------------------------------------
    //---
    //--- RangeStream
    //---
    //---------------------------------------------------------------
    RangeStream::RangeStream(IStream* stream, const Range& range)
        :stream_(stream)
        ,start_(range.start_)
        ,length_(range.length_)
    {
        CPPJSON_ASSERT(NULL != stream_);
        CPPJSON_ASSERT(0<=start_);
        CPPJSON_ASSERT(0<=length_);
    }

    off_t RangeStream::length() const
    {
        return length_;
    }

    s32 RangeStream::read(u8* dst)
    {
        CPPJSON_ASSERT(NULL != dst);
        off_t pos = stream_->tell();
        if(!stream_->seek(start_)){
            return 0;
        }
        s32 ret = stream_->read(dst, length_);
        if(!stream_->seek(pos)){
            return 0;
        }
        return ret;
    }

    s32 RangeStream::readAsString(Char* str)
    {
        CPPJSON_ASSERT(NULL != str);
        off_t pos = stream_->tell();
        if(!stream_->seek(start_)){
            return 0;
        }
        s32 ret = stream_->read(reinterpret_cast<u8*>(str), length_);
        if(!stream_->seek(pos)){
            return 0;
        }
        str[length_] = '\0';
        return ret;
    }

    s32 RangeStream::readAsString(String& str)
    {
        str.resize(static_cast<s32>(length_));
        off_t pos = stream_->tell();
        if(!stream_->seek(start_)){
            return 0;
        }
        Char* c = str.c_str();
        s32 ret = stream_->read(reinterpret_cast<u8*>(c), length_);
        if(!stream_->seek(pos)){
            return 0;
        }
        c[length_] = '\0';
        return ret;
    }

    s32 RangeStream::readAsInt(s32 defaultValue)
    {
        static const s32 MaxSize = 32;
        CPPJSON_ASSERT(length_<MaxSize);
        off_t pos = stream_->tell();
        if(!stream_->seek(start_)){
            return defaultValue;
        }
        Char buffer[MaxSize];
        s32 ret = stream_->read(reinterpret_cast<u8*>(buffer), length_);
        if(!stream_->seek(pos)){
            return defaultValue;
        }
        if(ret<=0){
            return defaultValue;
        }
        buffer[length_] = '\0';
        return atol(buffer);
    }

    f32 RangeStream::readAsFloat(f32 defaultValue)
    {
        static const s32 MaxSize = 32;
        CPPJSON_ASSERT(length_<MaxSize);
        off_t pos = stream_->tell();
        if(!stream_->seek(start_)){
            return defaultValue;
        }
        Char buffer[MaxSize];
        s32 ret = stream_->read(reinterpret_cast<u8*>(buffer), length_);
        if(!stream_->seek(pos)){
            return defaultValue;
        }
        if(ret<=0){
            return defaultValue;
        }
        buffer[length_] = '\0';
        return static_cast<f32>(atof(buffer));
    }

    //---------------------------------------------------------------
    //---
    //--- JSONReader
    //---
    //---------------------------------------------------------------
    JSONReader::JSONReader(IStream& istream, JSONEventHandler& handler)
        :flags_(0)
        ,istream_(istream)
        ,handler_(handler)
    {}

    JSONReader::~JSONReader()
    {
    }

    boolean JSONReader::read(s32 flags)
    {
        handler_.begin();
        flags_ = flags;
        Range value;
        if(!skipSpace()){
            if(istream_.isEOF()){
                return true;
            } else{
                onError();
                return false;
            }
        }
        s32 type = getValue(value);
        if(JSON_Error == type || (JSON_Array != type && JSON_Object != type)){
            return false;
        }
        while(!istream_.isEOF()){
            s32 c = istream_.get();
            if(c==EOF){
                break;
            }
            if(!isspace(c)){
                return false;
            }
        }
        handler_.root(type, RangeStream(&istream_, value));
        handler_.end();
        return true;
    }

    boolean JSONReader::skipSpace()
    {
        for(;;){
            s32 c = istream_.get();
            if(c<0){
                return false;
            }
            if(!isspace(c)){
                return EOF != istream_.unget(c);
            }
        }
    }

    boolean JSONReader::check(const s8* str)
    {
        CPPJSON_ASSERT(NULL != str);
        while('\0' != *str){
            s32 c = istream_.get();
            if(c<0){
                return false;
            }
            if(c != *str){
                return false;
            }
            ++str;
        }
        return true;
    }

    void JSONReader::setStart(Range& range)
    {
        range.start_ = istream_.tell();
        range.length_ = 0;
    }

    void JSONReader::setLength(Range& range)
    {
        range.length_ = istream_.tell()-range.start_-1;
    }

    s32 JSONReader::onError()
    {
        if(Flag_ErrorReported == (flags_&Flag_ErrorReported)){
            return JSON_Error;
        }
        flags_ |= Flag_ErrorReported;
        off_t pos = istream_.tell();
        if(!istream_.seek(0)){
            handler_.onError(-1, -1);
            return JSON_Error;
        }

        s32 line = 0;
        s32 count = 0;
        for(;;){
            s32 c = istream_.get();
            if(c == EOF){
                break;
            }
            if(c == '\r'){
                s32 c1 = istream_.get();
                if(c1 != '\n'){
                    if(EOF != c1){
                        istream_.unget(c1);
                    }
                }
                ++line;
                count = 0;
            }else if(c == '\n'){
                ++line;
                count = 0;
            }else{
                ++count;
            }
            if(istream_.tell() == pos){
                break;
            }
        }
        handler_.onError(line+1, count);
        return JSON_Error;
    }

    s32 JSONReader::getObject(Range& object)
    {
        setStart(object);
        handler_.beginObject();
        Range key;
        key.reset();
        boolean bKeyValue = false;
        boolean bSeparator = false;
        for(;;){
            if(!skipSpace()){
                return onError();
            }
            s32 c = istream_.get();
            switch(c){
            case JSONToken_OBJEnd:
                if(bSeparator && !bKeyValue){
                    return onError();
                }
                setLength(object);
                handler_.endObject(RangeStream(&istream_, object));
                return JSON_OK;

            case JSONToken_DQuote:
            {
                if(bKeyValue && !bSeparator){
                    return onError();
                }
                bKeyValue = true;
                bSeparator = false;
                if(JSON_Error == getString(key)){
                    return onError();
                }

                if(!skipSpace()){
                    return onError();
                }
                if(JSONToken_OBJColon != istream_.get()){
                    return onError();
                }
                if(!skipSpace()){
                    return onError();
                }
                Range value;
                s32 type = getValue(value);
                if(JSON_Error == type){
                    return JSON_Error;
                }
                handler_.keyValue(RangeStream(&istream_, key), type, RangeStream(&istream_, value));
            }
            break;
            case JSONToken_Separator:
                if(!bKeyValue){
                    return onError();
                }
                bKeyValue = false;
                bSeparator = true;
                break;
            default:
                return onError();
            }
        }
    }

    s32 JSONReader::getArray(Range& array)
    {
        setStart(array);
        handler_.beginArray();
        if(!skipSpace()){
            return onError();
        }
        s32 c = istream_.get();
        if(c<0){
            return onError();
        }
        switch(c){
        case JSONToken_ArrayEnd:
            setLength(array);
            handler_.endArray(RangeStream(&istream_, array));
            return JSON_OK;
        case JSONToken_Separator:
            return onError();
        default:
            break;
        }

        s32 prev = JSONToken_ArrayStart;
        for(;;){
            switch(c){
            case JSONToken_ArrayEnd:
                if(JSONToken_Separator == prev){
                    return onError();
                }
                setLength(array);
                handler_.endArray(RangeStream(&istream_, array));
                return JSON_OK;
            case JSONToken_Separator:
                if(JSONToken_Separator == prev){
                    return onError();
                }
                prev = JSONToken_Separator;
                break;
            default:
                if(JSONToken_Value == prev){
                    return onError();
                }
                prev = JSONToken_Value;
                if(EOF == istream_.unget(c)){
                    return onError();
                }
                Range value;
                s32 type = getValue(value);
                if(JSON_Error == type){
                    return JSON_Error;
                }
                handler_.value(type, RangeStream(&istream_, value));
                break;
            }
            if(!skipSpace()){
                return onError();
            }
            c = istream_.get();
            if(c<0){
                return onError();
            }
        }
    }

    s32 JSONReader::getValue(Range& value)
    {
        s32 c = istream_.get();
        if(c<0){
            return onError();
        }
        setStart(value);
        s32 type;
        switch(c)
        {
        case JSONToken_OBJStart:
            if(JSON_Error == getObject(value)){
                return onError();
            }
            type = JSON_Object;
            break;
        case JSONToken_ArrayStart:
            if(JSON_Error == getArray(value)){
                return onError();
            }
            type = JSON_Array;
            break;
        case JSONToken_DQuote:
            type = getString(value);
            break;
        case 't':
            if(!check(reinterpret_cast<const s8*>("rue"))){
                return onError();
            }
            type = JSON_True;
            setLength(value);
            break;
        case 'f':
            if(!check(reinterpret_cast<const s8*>("alse"))){
                return onError();
            }
            type = JSON_False;
            setLength(value);
            break;
        case 'n':
            if(!check(reinterpret_cast<const s8*>("ull"))){
                return onError();
            }
            type = JSON_Null;
            setLength(value);
            break;
        default:
            if(EOF == istream_.unget(c)){
                return onError();
            }
            type = getNumber(value);
            break;
        }
        return type;
    }

    s32 JSONReader::getString(Range& str)
    {
        setStart(str);
        for(;;){
            s32 c = istream_.get();
            if(c<0){
                return onError();
            }

            s32 n;
            if(0 == (c&0x80U)){//1byte
                switch(c){
                case JSONToken_DQuote:
                    setLength(str);
                    return JSON_String;
                case JSONToken_Escape:
                    c = istream_.get();
                    if(c<0){
                        return onError();
                    }
                    switch(c){
                    case JSONToken_DQuote:
                    case JSONToken_Escape:
                    case '/':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'u':
                        break;
                    default:
                        return onError();
                    }
                    break;
                case '\t':
                case '\n':
                    return onError();
                }
                continue;

            }else if(6 == (c>>5)){//2byte
                n=1;
            }else if(14 == (c>>4)){//3byte
                n=2;
            }else if(30 == (c>>3)){//4byte
                n=3;
            }else if(62 == (c>>2)){//5byte
                n=4;
            }else if(126 == (c>>1)){//6byte
                n=5;
            }else{
                return onError();
            }
            for(s32 i=0; i<n; ++i){
                c = istream_.get();
                if(c<0 || 2 != (c>>6)){
                    return onError();
                }
            }
        }
    }

    s32 JSONReader::getNumber(Range& number)
    {
        setStart(number);
        s32 type = JSON_Integer;
        s32 c = istream_.get();
        if(c<0){
            return onError();
        }
        if(c=='-'){
            c = istream_.get();
        }
        if(!isNumber(c)){
            return onError();
        }
        if(c != '0'){
            if(JSON_Error == getDigit()){
                return JSON_Error;
            }
        }
        c = istream_.get();
        if(c == '.'){
            c = istream_.get();
            if(!isNumber(c)){
                return onError();
            }
            if(JSON_Error == getDigit()){
                return JSON_Error;
            }
            type = JSON_Float;
            c = istream_.get();
        }

        if(c=='e' || c=='E'){
            c = istream_.get();
            if(c=='+'||c=='-'){
                c = istream_.get();
            }
            if(!isNumber(c)){
                return onError();
            }
            istream_.unget(c);
            c = getDigit(false);
            if(JSON_Error == c){
                return JSON_Error;
            }
            type = JSON_Float;
        }else if(c<0){
            return onError();
        }
        setLength(number);
        return (EOF != istream_.unget(c))? type : onError();
    }

    s32 JSONReader::getDigit(boolean unget)
    {
        s32 c = EOF;
        for(;;){
            c = istream_.get();
            if(c<0){
                return onError();
            }
            if(!isNumber(c)){
                if(unget){
                    return (EOF != istream_.unget(c))? c : onError();
                }
                break;
            }
        }
        return c;
    }

    boolean JSONReader::isNumber(s32 c) const
    {
        return ('0'<=c && c<='9');
    }
    boolean JSONReader::isOneToNine(s32 c) const
    {
        return ('1'<=c && c<='9');
    }
}
#endif //INC_CPPJSON_H_
