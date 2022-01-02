# Introduction
[![Build Status](https://travis-ci.org/taqu/cppjson.svg?branch=master)](https://travis-ci.org/taqu/cppjson)

A json parser with least memory allocations, never convert a json tree into other specific structures like STL.

## Concept

# Usage
# Limitations
The size of parsed files is limited by 4 GiB. The number of elements is limited by 4 giga.

# Tests
cppjson will pass tests of [JSON Parsing Test Suite](https://github.com/nst/JSONTestSuite)'s test_parsing. About tests of undefined behaviors, cppjson won't pass almost all of tests which are related with character enconfing matters.

# License
This software is distributed under the MIT license or the public domain, choose whichever you like.

