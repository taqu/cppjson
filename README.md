<!--- /** @page README --->
# Introduction
[![Build Status](https://travis-ci.org/taqu/cppjson.svg?branch=master)](https://travis-ci.org/taqu/cppjson)

Simple SAX(Simple API for XML) like JSON parser for C++.

# Usage
To create the implementation, put '#define CPPJSON_IMPLEMENTATION' before including "cppjson.h".  
Make a class which is derived from cppjson::JSONEventHandler, and pass that to cppjson::JSONReader.  
```cpp
#define CPPJSON_IMPLEMENTATION
#include "cppjson.h"

class EventNullHandler : public cppjson::JSONEventHandler
{
public:
    EventNullHandler(){}

    virtual void begin(){}
    virtual void end(){}
    virtual void beginObject(){}
    virtual void endObject(cppjson::RangeStream object){}

    virtual void beginArray(){}
    virtual void endArray(cppjson::RangeStream stream){}

    virtual void root(cppjson::s32 type, cppjson::RangeStream value){}
    virtual void value(cppjson::s32 type, cppjson::RangeStream v){}
    virtual void keyValue(cppjson::RangeStream key, cppjson::s32 type, cppjson::RangeStream value){}

    virtual void onError(cppjson::s32 line, cppjson::s32 charCount){}
};

int main(int argc, char** argv)
{
    FILE* file = cppjson::fopen_s(data, "rb");
    if(NULL == file){
        return;
    }
    cppjson::IFStream ifstream(file);
    EventNullHandler eventHandler;
    cppjson::JSONReader reader(ifstream, eventHandler);
    bool result = reader.read();
    fclose(file);
    return 0;
}
```
<!--- */ --->

