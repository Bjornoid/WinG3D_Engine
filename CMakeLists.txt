cmake_minimum_required(VERSION 3.16)

project(Wing3D_Engine)

# Great way to reduce compile times for large files like Gateware.h 
set(PRE_COMPILED
    ./Source/precompiled.h 
)

# CMake FXC shader compilation, add any shaders you want compiled here
set(VERTEX_SHADERS 
	# add vertex shader (.hlsl) files here
	./Shaders/VertexShader.hlsl
)

set(PIXEL_SHADERS 
	# add pixel shader (.hlsl) files here
	./Shaders/PixelShader.hlsl
)

# Create list of source files. 
file(
	GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
	./Source/*.h
	./Source/*.cpp
	./ThirdParty/gateware-main/Gateware.h
	./ThirdParty/flecs-master/flecs.h
    ./ThirdParty/flecs-master/flecs.c
	./ThirdParty/inifile-cpp-master/include/inicpp.h
	./defaults.ini # get some syntax highlighting
)
# The FLECS library core is written in C. If it tries to use the C++ pre-compiled header it will not compile.
set_source_files_properties(./ThirdParty/flecs-master/flecs.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
# Example: You can use this to remove files so later you can add them individually if needed.
#list(FILTER SOURCE_FILES EXCLUDE REGEX ".*main.*.cpp$")

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if (WIN32)
	# by default CMake selects "ALL_BUILD" as the startup project 
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
		 PROPERTY VS_STARTUP_PROJECT Wing3D_Engine)

	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VERTEX_SHADERS})
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PIXEL_SHADERS})
	source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCE_FILES})
endif()

add_executable (Wing3D_Engine 
		${SOURCE_FILES}
		${VERTEX_SHADERS}
		${PIXEL_SHADERS}
)

# using some c++17 filesystem features to grab file timestamps
# most code other code in the project only requires c++11
# we could remove this if we had to, but most compilers can do 17 these days
target_compile_features(Wing3D_Engine PUBLIC cxx_std_17)

# adding gateware.h and other librairies as a precompiled headers to reduce compile times
target_precompile_headers(Wing3D_Engine PRIVATE ${PRE_COMPILED})

set_source_files_properties( ${VERTEX_SHADERS} PROPERTIES 
        VS_SHADER_TYPE Vertex 
        VS_SHADER_MODEL 5.1
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "FXCompile" 
)

set_source_files_properties( ${PIXEL_SHADERS} PROPERTIES 
	    VS_SHADER_TYPE Pixel 
        VS_SHADER_MODEL 5.1
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "FXCompile"
)