#include "Diatom.h"
#include "Diatom-Storage.h"
#include <cassert>
#include <unistd.h>
#include <vector>

#define p_assert(x) do {             \
		printf("%60s", #x);    \
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
	p_assert(tl_1.n_descendants() == 4);
	p_assert(tl_1["monkeys"].isNumber());
	p_assert(tl_1["custard"].isString());
	p_assert(tl_1["bananalike"].isBool());
	tl_1["subtable"] = Diatom();
	tl_1["subtable"]["pears"] = double(7);
	p_assert(tl_1["subtable"].isTable());
	p_assert(tl_1["subtable"]["pears"].isNumber());
	
	
	printf("- Testing descendant ordering\n");
	Diatom tl_o;
	tl_o["n10"];
	tl_o["n4"];
	tl_o["n20a"];
	tl_o["n20"];
	tl_o["n1"];
	tl_o["m3"];
	std::vector<std::string> exp = {
		"n10", "n4", "n20a", "n20", "n1", "m3"
	};
	int i = 0;
	tl_o.each_descendant([&](std::string s, Diatom &d) {
		p_assert(s == exp[i++]);
	});
	p_assert(i == exp.size());
		
	
	printf("- Testing table copying\n");
	Diatom russians;
	russians["nikolai"] = 12.4;
	russians["mikhail"] = "Gorbachev";
	russians["scientists"] = Diatom();
	russians["scientists"]["pavlov"] = "Dogs";
	
	Diatom r2 = russians;
	p_assert(russians.isTable());
	p_assert(r2.isTable());
	p_assert(russians.n_descendants() == r2.n_descendants());
	p_assert(r2["nikolai"].isNumber());
	p_assert(r2["mikhail"].isString());
	p_assert(r2["nikolai"].number_value() == 12.4);
	p_assert(r2["mikhail"].str_value() == "Gorbachev");
	p_assert(r2["scientists"].isTable());
	p_assert(r2["scientists"].n_descendants() == 1);
	p_assert(r2["scientists"]["pavlov"].str_value() == "Dogs");
	
	
	printf("- Testing .diatom serialization\n");
	Diatom d_ser;
	Diatom nikolai2;
	{ /* Serialize to tmp file */
		std::string ser = diatomToString(russians, "testykins");
		FILE *fp = fopen("/tmp/diatom-test.diatom", "w");
		p_assert(fp);
		int blah = fputs(ser.c_str(), fp);
		p_assert(blah >= 0 && blah != EOF);
		fclose(fp);
	}
	d_ser = diatomFromFile("/tmp/diatom-test.diatom")["testykins"];
	p_assert(d_ser.isTable());
	p_assert(d_ser["nikolai"].isNumber());
	p_assert(d_ser["mikhail"].isString());
	p_assert(d_ser["nikolai"].number_value() == 12.4);
	p_assert(d_ser["mikhail"].str_value() == "Gorbachev");
	p_assert(d_ser["scientists"].isTable());
	p_assert(d_ser["scientists"].n_descendants() == 1);
	p_assert(d_ser["scientists"]["pavlov"].str_value() == "Dogs");
	unlink("/tmp/diatom-test.diatom");
	
	printf("\n");
}
