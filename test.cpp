// clang++ -std=c++11    \
// 	  test.cpp LuaObj.cpp                   \
// 	  -I ../../L-Serialize/lua-5.2.1/src/   \
// 	  -L ../../L-Serialize/lua-5.2.1/       \
// 	  -lLua-x86_64-O3

#include "LuaObj.h"
#include "lua.hpp"
#include <cassert>


#define p_assert(x) do {        \
		printf("TEST: %35s", #x);    \
		assert(x);              \
		printf(" - PASS :)\n"); \
	} while (false)


void tests(LuaObj &myObj) {
	p_assert(myObj.isTable());
	p_assert(myObj.descendants.size() == 3);

	LuaObj l_num = myObj["a_number"];
	p_assert(l_num.isNumber());
	p_assert(l_num.number_value == 5.1);
	
	LuaObj l_bool = myObj["a_bool"];
	p_assert(l_bool.isBool());
	p_assert(l_bool.bool_value == true);
	
	LuaObj l_str = myObj["a_string"];
	p_assert(l_str.isString());
	p_assert(l_str.str_value == "ninja turtles");
}


int main() {
	
	printf("Testing LuaObjs loaded from file...\n");
	LuaObj o1("test.lua", "a_table");
	tests(o1);
	
	printf("\nTesting LuaObjs loaded from State...\n");
    lua_State *L;
    if (LuaObj::loadLuaFile("test.lua", &L)) {
        lua_getglobal(L, "a_table");
        LuaObj o2 = LuaObj(L);
		tests(o2);
        lua_pop(L, 1);
		
		// Check we have used the lua stack correctly
		lua_getglobal(L, "another_table");
		LuaObj o3 = LuaObj(L);
		lua_pop(L, 1);
		
		lua_close(L);
    }
	
	return 0;
}

