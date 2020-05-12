// clang++ -std=c++11      \
// 	  test.cpp LuaObj.cpp  \
//    -DLUAOBJ_PRINT

#include "LuaObj.h"
#include <cassert>

#define p_assert(x) do {        \
		printf("TEST: %38s", #x);    \
		assert(x);              \
		printf(" - PASS :)\n"); \
	} while (false)

#define p_header(s) do {                               \
		for (int i=0; s[i] != '\0'; ++i) printf("*");  \
		printf("********\n");                          \
		printf("**  %s  **\n", s);                     \
		for (int i=0; s[i] != '\0'; ++i) printf("*");  \
		printf("********\n");                          \
	} while (false)


void testLuaObj();

int main() {
	testLuaObj();
	return 0;
}


void testLuaObj() {
	p_header("Testing LuaObj");
	
	printf("- Testing Numbers\n");
	LuaObj nl_1((double) 5), nl_2(7.42);
	p_assert(nl_1.isNumber() && nl_2.isNumber());
	p_assert(nl_1.number_value() == 5.0);
	p_assert(nl_2.number_value() == 7.42);
	
	printf("- Testing Booleans\n");
	LuaObj bl(false);
	p_assert(bl.isBool());
	p_assert(bl.bool_value() == false);
	
	printf("- Testing Strings\n");
	std::string s = "Who would fardels bear";
	LuaObj sl_1(s), sl_2(s.c_str());
	p_assert(sl_1.isString() && sl_2.isString());
	p_assert(sl_1.str_value() == s);
	p_assert(sl_2.str_value() == s);
	
	printf("- Testing Nil\n");
	LuaObj nill = LuaObj::NilObject();
	p_assert(nill.isNil());	
	
	printf("- Testing Tables\n");
	LuaObj tl_1;
	p_assert(tl_1.isTable());
	p_assert(tl_1["penguins"].isNil());
	tl_1["monkeys"] = (double) 12;
	tl_1["custard"] = "lemons";
	p_assert(tl_1.descendants().size() == 3);
	p_assert(tl_1["monkeys"].isNumber());
	p_assert(tl_1["custard"].isString());
	
	printf("- Testing table copyiing\n");
	tl_1["russians"] = LuaObj();
	tl_1["russians"]["nikolai"] = 12.4;
	tl_1["russians"]["mikhail"] = "Gorbachev";
	p_assert(tl_1["russians"].descendants().size() == 2);
	
	LuaObj tl_2 = tl_1;
	p_assert(tl_2["russians"]["nikolai"].number_value() == 12.4);
	p_assert(&(tl_2["russians"]["mikhail"]) != &(tl_1)["russians"]["mikhail"]);
	
}
