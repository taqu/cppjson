#include "cppjson.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#endif
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Test
{
    enum class JsonType
    {
        Accept,
        Reject,
        Undefined,
    };

    JsonType type_;
    std::string path_;
};

bool start_with(const std::string& filename, char c)
{
    if(filename.length() <= 0) {
        return false;
    }
    return c == filename[0];
}

#ifndef _WIN32
int ext(const char* str, const char* ext)
{
    const char* s = str;
    while('\0' != *s){
        if('.' == *s){
            ++s;
            break;
        }
        ++s;
    }
    return strcmp(s, ext);
}

int filter(const struct dirent* dir)
{
    if(DT_REG != dir->d_type){
        return 0;
    }
    return 0 == ext(dir->d_name, "json")? 1 : 0;
}
#endif

void gather(std::vector<Test>& result, const char* directory)
{
#ifdef _WIN32
    std::string path = directory;
    path += "*.json";
    WIN32_FIND_DATA findFileData;
    HANDLE handle = FindFirstFileA(path.c_str(), &findFileData);
    if(CPPJSON_NULL == handle) {
        return;
    }
    for(;;) {
        if(FALSE == FindNextFileA(handle, &findFileData)) {
            break;
        }
        Test test;
        test.path_ = findFileData.cFileName;
        if(start_with(test.path_, 'i')) {
            test.type_ = Test::JsonType::Undefined;
        } else if(start_with(test.path_, 'y')) {
            test.type_ = Test::JsonType::Accept;
        } else if(start_with(test.path_, 'n')) {
            test.type_ = Test::JsonType::Reject;
        }
        result.push_back(test);
    }
    FindClose(handle);
#else
    struct dirent** namelist = nullptr;
    int n = scandir(directory, &namelist, filter, alphasort);
    for(int i=0; i<n; ++i){
        Test test;
        test.path_ = namelist[i]->d_name;
        if(start_with(test.path_, 'i')) {
            test.type_ = Test::JsonType::Undefined;
        } else if(start_with(test.path_, 'y')) {
            test.type_ = Test::JsonType::Accept;
        } else if(start_with(test.path_, 'n')) {
            test.type_ = Test::JsonType::Reject;
        }
        result.push_back(test);
        free(namelist[i]);
    }
    free(namelist);
#endif
}

struct Data
{
    size_t size_;
    void* data_;
};
Data read(const std::string& path)
{
    std::ifstream file;
    file.open(path.c_str(), std::ios::binary);
    if(!file.is_open()) {
        return {0, CPPJSON_NULL};
    }
    Data data;
    file.seekg(0, std::ios::end);
    data.size_ = file.tellg();
    file.seekg(0, std::ios::beg);
    data.data_ = ::malloc(data.size_);
    file.read(reinterpret_cast<char*>(data.data_), data.size_);
    file.close();
    return data;
}

void test(const std::vector<Test>& tests, const char* directory)
{
    cppjson::JsonParser parser;
    std::string path;
    size_t skip = 0;
    for(size_t i = 0; i < tests.size(); ++i) {
        if(i < skip) {
            continue;
        }
        const Test& test = tests[i];
        path = directory;
        path += test.path_;
        Data data = read(path);
        if(CPPJSON_NULL == data.data_) {
            continue;
        }
        char* begin = reinterpret_cast<char*>(data.data_);
        bool result = parser.parse(begin, begin + data.size_);

        switch(test.type_) {
        case Test::JsonType::Accept:
            if(!result) {
                abort();
            }
            break;
        case Test::JsonType::Reject:
            if(result) {
                abort();
            }
            break;
        case Test::JsonType::Undefined:
#ifdef _DEBUG
            if(result) {
                std::cout << "accept" << std::endl;
            } else {
                std::cout << "reject" << std::endl;
            }
#endif
            break;
        }
        ::free(data.data_);
    }
}

int main(void)
{
    { // Test json parser
        std::string directory = "../../test/JSONTestSuite/test_parsing/";
        std::vector<Test> tests;
        gather(tests, directory.c_str());
        test(tests, directory.c_str());
    }
    return 0;
}
