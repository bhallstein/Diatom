#include "Diatom.h"
#include "Diatom-Lua.h"
#include "Diatom-Storage.h"
#include <cassert>
#include <unistd.h>
#include <vector>

#define p_assert(x) do {             \
		printf("TEST: %38s", #x);    \
		assert(x);                   \
		printf(" - PASS :)\n");      \
	} while (false)

#define p_header(s) do {                               \
		for (int i=0; s[i] != '\0'; ++i) printf("*");  \
		printf("********\n");                          \
		printf("**  %s  **\n", s);                     \
		for (int i=0; s[i] != '\0'; ++i) printf("*");  \
		printf("********\n");                          \
	} while (false)


void testDiatom();

int main() {
	testDiatom();
	return 0;
}


void testDiatom() {
	p_header("Testing Diatom");
	
	printf("- Testing numbers\n");
	Diatom nl_1((double) 5), nl_2(7.42);
	p_assert(nl_1.isNumber() && nl_2.isNumber());
	p_assert(nl_1.number_value() == 5.0);
	p_assert(nl_2.number_value() == 7.42);
	
	printf("- Testing booleans\n");
	Diatom bl(false);
	p_assert(bl.isBool());
	p_assert(bl.bool_value() == false);
	
	printf("- Testing strings\n");
	std::string s = "Who would fardels bear";
	Diatom sl_1(s), sl_2(s.c_str());
	p_assert(sl_1.isString() && sl_2.isString());
	p_assert(sl_1.str_value() == s);
	p_assert(sl_2.str_value() == s);
	
	printf("- Testing nil\n");
	Diatom nil1 = Diatom::NilObject();
	Diatom nil2 = Diatom::NilObject();
	p_assert(nil1.isNil());
	p_assert(&nil1 != &nil2);
	
	printf("- Testing tables\n");
	Diatom tl_1;
	p_assert(tl_1.isTable());
	p_assert(tl_1["penguins"].isNil());
	tl_1["monkeys"] = (double) 12;
	tl_1["custard"] = "lemons";
	tl_1["bananalike"] = false;
	p_assert(tl_1.descendants().size() == 4);
	p_assert(tl_1["monkeys"].isNumber());
	p_assert(tl_1["custard"].isString());
	p_assert(tl_1["bananalike"].isBool());
	
	printf("- Testing descendant ordering\n");
	Diatom tl_o;
	tl_o["n10"];
	tl_o["n4"];
	tl_o["n20a"];
	tl_o["n20"];
	tl_o["n1"];
	tl_o["m3"];
	std::vector<std::string> exp = {
		"m3", "n1", "n4", "n10", "n20", "n20a"
	};
	int i = 0;
	for (auto &desc : tl_o.descendants())
		p_assert(desc.first == exp[i++]);
		
	
	printf("- Testing table copying\n");
	tl_1["russians"] = Diatom();
	tl_1["russians"]["nikolai"] = 12.4;
	tl_1["russians"]["mikhail"] = "Gorbachev";
	Diatom tl_2 = tl_1;
	Diatom nikolai = tl_2["russians"]["nikolai"];
	Diatom mikhail = tl_2["russians"]["mikhail"];
	Diatom *p_mikhail_1 = &(tl_1["russians"]["mikhail"]);
	Diatom *p_mikhail_2 = &(tl_2["russians"]["mikhail"]);
	p_assert(nikolai.number_value() == 12.4);
	p_assert(mikhail.str_value() == "Gorbachev");
	p_assert(p_mikhail_1 != p_mikhail_2);
	
	printf("- Testing lua serialization\n");
	tl_1["francais"] = Diatom();
	tl_1["francais"]["jacques"] = "Chirac";
	{ /* Serialize to tmp file */
		std::string ser = std::string("testykins = ") + diatomToLua(tl_1);
	    FILE *fp = fopen("/tmp/diatom-test.lua", "w");
		p_assert(fp != NULL);
        fputs(ser.c_str(), fp);
        fclose(fp);
	}
	Diatom d_ser = luaToDiatom("/tmp/diatom-test.lua", "testykins");
	unlink("/tmp/diatom-test.lua");
	Diatom nikolai2 = d_ser["russians"]["nikolai"];
	p_assert(nikolai2.number_value() == 12.4);
	
	printf("- Testing .diatom serialization\n");
	{ /* Serialize to tmp file */
		std::string ser = diatomToString(tl_1, "testykins");
		// printf("%s\n", ser.c_str());
		FILE *fp = fopen("/tmp/diatom-test.diatom", "w");
		p_assert(fp);
		fputs(ser.c_str(), fp);
		fclose(fp);
	}
	d_ser = diatomFromFile("/tmp/diatom-test.diatom")["testykins"];
	unlink("/tmp/diatom-test.diatom");
	nikolai2 = d_ser["russians"]["nikolai"];
	p_assert(nikolai2.number_value() == 12.4);
	
	printf("\n");
}
