#include "config.h"
#if 0
#include <Windows.h>
#include <iostream>

namespace
{
    bool isFile(const WIN32_FIND_DATA& data)
    {
        static const DWORD checks[] =
        {
            FILE_ATTRIBUTE_HIDDEN,
            FILE_ATTRIBUTE_SYSTEM,
            FILE_ATTRIBUTE_DIRECTORY,
            FILE_ATTRIBUTE_ENCRYPTED,
            FILE_ATTRIBUTE_TEMPORARY,
            FILE_ATTRIBUTE_SPARSE_FILE,
        };
        static const int Num = sizeof(checks)/sizeof(DWORD);
        for(int i=0; i<Num; ++i){
            DWORD check = checks[i] & data.dwFileAttributes;
            if(check != 0){
                return false;
            }
        }
        return true;
    }
}
#endif

TEST_CASE("test_Json00.json can be loaded", "[Json00]"){
    static const char* data = DATA_ROOT"test_Json00.json";

#if 1
    SECTION("load"){
        FILE* file = cppjson::fopen_s(data, "rb");
        if(NULL == file){
            return;
        }
        cppjson::IFStream ifstream(file);
        EventHandler eventHandler;
        cppjson::JSONReader reader(ifstream, eventHandler);
        bool result = reader.read();
        fclose(file);
        REQUIRE(result);
    }

#else
    SECTION("test_data_fail"){
        WIN32_FIND_DATA findData;
        std::string datapath = "..\\data\\jsonchecker\\"; 
        std::string path = datapath;
        path += "fail*.json";
        HANDLE handle = FindFirstFile(path.c_str(), &findData);
        if(INVALID_HANDLE_VALUE == handle){
            return;
        }
        std::string name;
        FILE* file;
        for(;;){
            if(isFile(findData)){
                path = datapath;
                path += findData.cFileName;
                file = fopen(path.c_str(), "rb");
                if(NULL != file){
                    cppjson::IFStream ifstream(file);
                    EventNullHandler eventHandler;
                    cppjson::JSONReader reader(ifstream, eventHandler);
                    bool result = reader.read();
                    const char* r = (result)?"true":"false";
                    std::cout << findData.cFileName << " " << r << std::endl;
                    fclose(file);
                    REQUIRE_FALSE(result);
                }
            }
            if(FALSE == FindNextFile(handle, &findData)){
                break;
            }
        }
        FindClose(handle);
    }

    SECTION("test_data_pass"){
        WIN32_FIND_DATA findData;
        std::string datapath = "..\\data\\jsonchecker\\"; 
        std::string path = datapath;
        path += "pass*.json";
        HANDLE handle = FindFirstFile(path.c_str(), &findData);
        if(INVALID_HANDLE_VALUE == handle){
            return;
        }
        std::string name;
        FILE* file;
        for(;;){
            if(isFile(findData)){
                path = datapath;
                path += findData.cFileName;
                file = fopen(path.c_str(), "rb");
                if(NULL != file){
                    cppjson::IFStream ifstream(file);
                    EventNullHandler eventHandler;
                    cppjson::JSONReader reader(ifstream, eventHandler);
                    bool result = reader.read();
                    const char* r = (result)?"true":"false";
                    std::cout << findData.cFileName << " " << r << std::endl;
                    fclose(file);
                    REQUIRE(result);
                }
            }
            if(FALSE == FindNextFile(handle, &findData)){
                break;
            }
        }
        FindClose(handle);
    }
#endif
}
