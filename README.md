# Introduction
[![Build Status](https://travis-ci.org/taqu/cppjson.svg?branch=master)](https://travis-ci.org/taqu/cppjson)

A simple json parser.

# Usage
To create the implementation, put '#define CPPJSON_IMPLEMENTATION' before including "cppjson.h".  
Make a class which is derived from cppjson::JSONEventHandler, and pass that to cppjson::JSONReader.  
```cpp
#define CPPJSON_IMPLEMENTATION
#include "cppjson.h"

int main(int argc, char** argv)
{
    FILE* file = cppjson::fopen_s(data, "rb");
    if(NULL == file){
        return;
    }
    fclose(file);
    return 0;
}
```

# License
This software is distributed under two licenses, MIT License or Public Domain, choose whichever you like.

