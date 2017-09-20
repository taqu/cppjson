#include "config.h"

TEST_CASE("test_Json00.json can be loaded", "[Json00]"){
    static const char* data = DATA_ROOT"test_Json00.json";

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
}
