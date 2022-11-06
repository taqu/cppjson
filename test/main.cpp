
#define CPPJSON_IMPLEMENTATION
#include "cppjson.h"

#include <stdio.h>
#include <string>
#include <vector>
struct File
{
	enum class Type
    {
		Valid,
		Invalid,
		Either,
    };
	Type type_;
	std::string path_;
};

#ifdef _WIN32
#include <Windows.h>
#include <fileapi.h>
void gather(std::vector<File>& files, const char* directory, const char* pattern)
{
	files.clear();
	WIN32_FIND_DATAA findFileData;
	std::string path = directory;
	path += pattern;
	HANDLE handle = FindFirstFileA(path.c_str(), &findFileData);
	if(INVALID_HANDLE_VALUE != handle){
        do {
			if(0 == strcmp(findFileData.cFileName, ".") || 0 == strcmp(findFileData.cFileName, "..")){
				continue;
			}
			findFileData.cFileName;
			path = directory;
			path += findFileData.cFileName;
			File file;
			file.path_ = directory;
			file.path_ += findFileData.cFileName;
			if(0 == strncmp(findFileData.cFileName, "i_", 2)){
				file.type_ = File::Type::Either;
			}else if(0 == strncmp(findFileData.cFileName, "y_", 2)){
				file.type_ = File::Type::Valid;
			}else if(0 == strncmp(findFileData.cFileName, "n_", 2)){
				file.type_ = File::Type::Invalid;
			}

			files.push_back(std::move(file));
        }while(TRUE == FindNextFileA(handle, &findFileData));
	}
	FindClose(handle);
}
#endif

void test(const std::vector<File>& files)
{
	cppjson::JsonReader reader;
	size_t skip = 0;
	for(size_t i=skip; i<files.size(); ++i){
		const File& file = files[i];
		if(file.type_ != File::Type::Either){
			continue;
		}
		FILE* f = fopen(file.path_.c_str(), "rb");
		if(NULL == f){
			continue;
		}
		struct stat s;
		fstat(fileno(f), &s);
		size_t size = s.st_size;
		char* data = (char*)::malloc(size);
		if(fread(data, size, 1, f)<=0){
			fclose(f);
			::free(data);
			continue;
		}
		fclose(f);
		printf("[%zd] %s", i, file.path_.c_str());
		bool result = reader.parse(data, data+size);
		switch(file.type_){
        case File::Type::Valid:
			assert(result);
			printf("  %d\n", result);
			break;
        case File::Type::Invalid:
			assert(!result);
			printf("  %d\n", result);
			break;
		case File::Type::Either:
			printf("  %d\n", result);
			break;
        default:
			assert(false);
			break;
		}
        ::free(data);
	}
}

void traverse_object(cppjson::JsonProxy proxy);
void traverse_array(cppjson::JsonProxy proxy);
void traverse_keyvalue(cppjson::JsonProxy proxy);
void traverse_string(cppjson::JsonProxy proxy);
void traverse_number(cppjson::JsonProxy proxy);
void traverse_true(cppjson::JsonProxy proxy);
void traverse_false(cppjson::JsonProxy proxy);
void traverse_null(cppjson::JsonProxy proxy);

void traverse(cppjson::JsonProxy proxy)
{
	using namespace cppjson;
    switch(proxy.type()) {
    case JsonType::Object:
        traverse_object(proxy);
        break;
    case JsonType::Array:
        traverse_array(proxy);
        break;
    case JsonType::KeyValue:
        traverse_keyvalue(proxy);
        break;
    case JsonType::String:
        traverse_string(proxy);
        break;
    case JsonType::Number:
        traverse_number(proxy);
        break;
    case JsonType::True:
        traverse_true(proxy);
        break;
    case JsonType::False:
        traverse_false(proxy);
        break;
    case JsonType::Null:
        traverse_null(proxy);
        break;
	}
}

void traverse_object(cppjson::JsonProxy proxy)
{
    using namespace cppjson;
    printf("{");
	for (JsonProxy i = proxy.begin(); i; i = i.next()) {
        JsonType type = i.type();
        char key[128];
        i.key().getString(key);
        printf("%s: ", key);
        traverse(i.value());
        printf(",\n");
	}
    printf("}");
}

void traverse_array(cppjson::JsonProxy proxy)
{
    using namespace cppjson;
    printf("[");
    for(JsonProxy i = proxy.begin(); i; i = i.next()) {
        traverse(i);
        printf(",");
    }
    printf("]");
}

void traverse_keyvalue(cppjson::JsonProxy proxy)
{
    char key[128];
    proxy.key().getString(key);
    printf("%s: ", key);
    traverse(proxy.value());
}

void traverse_string(cppjson::JsonProxy proxy)
{
    char value[128];
    proxy.getString(value);
    printf("%s", value);
}

void traverse_number(cppjson::JsonProxy proxy)
{
    char value[64];
    proxy.getString(value);
    printf("%s", value);
}

void traverse_true(cppjson::JsonProxy proxy)
{
    char value[16];
    proxy.getString(value);
    printf("%s", value);
}

void traverse_false(cppjson::JsonProxy proxy)
{
    char value[16];
    proxy.getString(value);
    printf("%s", value);
}

void traverse_null(cppjson::JsonProxy proxy)
{
    char value[16];
    proxy.getString(value);
    printf("%s", value);
}

void test(const char* path)
{
    FILE* f = fopen(path, "rb");
    if(NULL == f) {
        return;
    }
    struct stat s;
    fstat(fileno(f), &s);
    size_t size = s.st_size;
    char* data = (char*)::malloc(size);
    if(fread(data, size, 1, f) <= 0) {
        fclose(f);
        ::free(data);
        return;
    }
    fclose(f);
    cppjson::JsonReader reader;
    bool result = reader.parse(data, data + size);
    assert(result);
    cppjson::JsonProxy proxy = reader.root();
    traverse(proxy);
    ::free(data);
}

int main(void)
{
	std::vector<File> files;
	gather(files, "../JSONTestSuite/test_parsing/", "*.json");
	test(files);
	
	test("../test00.json");
	return 0;
}

