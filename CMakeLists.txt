cmake_minimum_required(VERSION 3.10)

project(lua-qoi)

#find_package(Lua51 REQUIRED)

# For other Lua versions, replace 5.1 with 5.2, 5.3 or 5.4
find_package(Lua 5.1 EXACT REQUIRED)

include_directories(${LUA_INCLUDE_DIR})

#link_directories(${LUA_LIBRARY_DIR})

#link_libraries(lua51)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_library(lua-qoi SHARED lua-qoi.c)
target_link_libraries(lua-qoi ${LUA_LIBRARIES})