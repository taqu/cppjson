/**
@file common.cpp
@author t-sakai
@date 2017/09/21 create
*/
#define CPPJSON_IMPLEMENTATION
#include "common.h"
#include <iostream>

EventHandler::EventHandler()
    :indent_(0)
{
}

void EventHandler::begin()
{
}

void EventHandler::end()
{
}

void EventHandler::beginObject()
{
    printIndent();
    std::cout << "beginObject" << std::endl;
    ++indent_;
}

void EventHandler::endObject(cppjson::RangeStream /*object*/)
{
    --indent_;
    printIndent();
    std::cout << "endObject" << std::endl;
}

void EventHandler::beginArray()
{
    printIndent();
    std::cout << "beginArray" << std::endl;
    ++indent_;
}

void EventHandler::endArray(cppjson::RangeStream /*stream*/)
{
    --indent_;
    printIndent();
    std::cout << "endArray" << std::endl;
}

void EventHandler::root(cppjson::s32 /*type*/, cppjson::RangeStream /*value*/)
{
}

void EventHandler::value(cppjson::s32 type, cppjson::RangeStream v)
{
    printIndent();
    int length = v.length();
    length += 1;
    char* str = (char*)malloc(length);

    switch(type)
    {
    case cppjson::JSON_Object:
        std::cout << "object" << std::endl;
        break;
    case cppjson::JSON_Array:
        std::cout << "array" << std::endl;
        break;
    case cppjson::JSON_String:
    {
        v.readAsString(str);
        std::cout << str << std::endl;
    }
    break;
    case cppjson::JSON_Integer:
    {
        v.readAsString(str);
        std::cout << str << ":int" << std::endl;
    }
    break;
    case cppjson::JSON_Float:
    {
        v.readAsString(str);
        std::cout << str << ":float" << std::endl;
    }
    break;
    case cppjson::JSON_True:
        std::cout << "true" << std::endl;
        break;
    case cppjson::JSON_False:
        std::cout << "false" << std::endl;
        break;
    case cppjson::JSON_Null:
        std::cout << "null" << std::endl;
        break;
    }
    free(str);
}

void EventHandler::keyValue(cppjson::RangeStream key, cppjson::s32 type, cppjson::RangeStream value)
{
    printIndent();
    int length = key.length()<value.length()? value.length() : key.length();
    length += 1;
    char* str = (char*)malloc(length);

    key.readAsString(str);
    std::cout << str << ":";

    switch(type)
    {
    case cppjson::JSON_Object:
        std::cout << "object" << std::endl;
        break;
    case cppjson::JSON_Array:
        std::cout << "array" << std::endl;
        break;
    case cppjson::JSON_String:
    {
        value.readAsString(str);
        std::cout << str << std::endl;
    }
    break;
    case cppjson::JSON_Integer:
    {
        value.readAsString(str);
        std::cout << str << ":int" << std::endl;
    }
    break;
    case cppjson::JSON_Float:
    {
        value.readAsString(str);
        std::cout << str << ":float" << std::endl;
    }
    break;
    case cppjson::JSON_True:
        std::cout << "true" << std::endl;
        break;
    case cppjson::JSON_False:
        std::cout << "false" << std::endl;
        break;
    case cppjson::JSON_Null:
        std::cout << "null" << std::endl;
        break;
    }
    free(str);
}

void EventHandler::printIndent()
{
    for(cppjson::s32 i=0; i<indent_; ++i){
        std::cout << ' ';
    }
}

void EventHandler::onError(cppjson::s32 line, cppjson::s32 charCount)
{
    std::cout << "error at line " << line << " : char " << charCount << std::endl;
}
