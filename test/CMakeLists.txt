cmake_minimum_required(VERSION 3.6)

set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")

set(ProjectName TestCppJson)
project(${ProjectName})

set(DATA_ROOT "./")
set(TEST_DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/")

if(MSVC)
set(CPPJSON_DATA_ROOT ".${DATA_ROOT}")
else()
set(CPPJSON_DATA_ROOT "${DATA_ROOT}")
endif()

set(SOURCES "main.cpp;common.cpp")

set(JSON_TESTS
    "Json00"
    "Json01")

foreach(JSON_TEST IN LISTS JSON_TESTS)
    if(EXISTS "${TEST_DATA_PATH}test_${JSON_TEST}.json")
        string(TOUPPER ${JSON_TEST} JSON_UPPER)
        set(TEST_${JSON_UPPER} ON)
        set(SOURCES "${SOURCES};test_${JSON_TEST}.cpp")
    endif()
endforeach()

# Set configuration
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.h" NEWLINE_STYLE LF)

include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})

source_group("include" FILES ${HEADERS})
source_group("src" FILES ${SOURCES})

set(FILES ${HEADERS} ${SOURCES})

set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

add_executable(${ProjectName} ${FILES})

if(MSVC)
    set(DEFAULT_CXX_FLAGS "/DWIN32 /D_WINDOWS /D_MBCS /W4 /WX- /nologo /fp:precise /Zc:wchar_t /TP /Gd")
    if("1800" VERSION_LESS MSVC_VERSION)
        set(DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} /EHsc")
    endif()

    set(CMAKE_CXX_FLAGS "${DEFAULT_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /DMU3_EXPORTS /MTd /Zi /Ob0 /Od /RTC1 /Gy /GR- /GS /Gm-")
    set(CMAKE_CXX_FLAGS_RELEASE "/DMU3_EXPORTS /MT /O2 /GL /GR- /DNDEBUG")

elseif(UNIX)
    set(DEFAULT_CXX_FLAGS "-Wall -O2 -std=c++11")
    set(CMAKE_CXX_FLAGS "${DEFAULT_CXX_FLAGS}")
elseif(APPLE)
endif()

set_target_properties(${ProjectName} PROPERTIES OUTPUT_NAME_DEBUG "${ProjectName}" OUTPUT_NAME_RELEASE "${ProjectName}")
