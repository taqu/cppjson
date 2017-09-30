#ifndef INC_COMMON_H__
#define INC_COMMON_H__
/**
@file common.h
@author t-sakai
@date 2017/09/21 create
*/
#include "../cppjson.h"

class EventHandler : public cppjson::JSONEventHandler
{
public:
    EventHandler();

    virtual void begin();
    virtual void end();
    virtual void beginObject();
    virtual void endObject(cppjson::RangeStream object);

    virtual void beginArray();
    virtual void endArray(cppjson::RangeStream stream);

    virtual void root(cppjson::s32 type, cppjson::RangeStream value);
    virtual void value(cppjson::s32 type, cppjson::RangeStream v);
    virtual void keyValue(cppjson::RangeStream key, cppjson::s32 type, cppjson::RangeStream value);

    virtual void onError(cppjson::s32 line, cppjson::s32 charCount);
private:
    void printIndent();
    cppjson::s32 indent_;
};

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
#endif //INC_COMMON_H__
