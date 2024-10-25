cmake_minimum_required(VERSION 3.20.0)

project(myApp
	VERSION 0.0
	DESCRIPTION "Stub"
	LANGUAGES CXX)

add_executable(myApp
	src/main.cpp)

target_compile_features(myApp PRIVATE cxx_std_20)


# add compile defs with -D
#target_compile_definitions(myApp PRIVATE DEFINITIONHERE)

# Classic compile options NOT CROSS PLATFORM
target_compile_options(myApp PRIVATE -Wall -Wextra -Wpedantic)




