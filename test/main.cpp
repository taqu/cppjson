
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

int main(void)
{
	std::vector<File> files;
	gather(files, "../JSONTestSuite/test_parsing/", "*.json");
	test(files);
	return 0;
}

