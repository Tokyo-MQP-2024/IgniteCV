cmake_minimum_required(VERSION 3.20.0)

project(myApp
	VERSION 0.0
	DESCRIPTION "Stub"
	LANGUAGES CXX)

# Compile commands for nvim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# More elegant way of compiling the source folder
file(GLOB_RECURSE sources
	"${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# add compile defs with -D
#target_compile_definitions(myApp PRIVATE DEFINITIONHERE)


# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include for all targets
include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(myApp ${sources})

# Include directories
target_include_directories(myApp PUBLIC "${CMAKE_SOURCE_DIR}/include")

# Target libraries (.a files)
target_link_libraries(myApp ${OpenCV_LIBS})

target_compile_features(myApp PRIVATE cxx_std_20)







if(UNIX AND NOT APPLE)
	# Classic compile options NOT CROSS PLATFORM
	target_compile_options(myApp PRIVATE -Wall -Wextra -Wpedantic)
endif()




