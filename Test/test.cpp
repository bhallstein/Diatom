#include "_test.h"
#include "../Diatom.h"
#include "../DiatomSerialization.h"
#include <iostream>


using Token = _DiatomSerialization::Token;


void printtype(Diatom &d) {
  printf("type: ");
  switch (d.type) {
    case Diatom::Type::Number: { printf("number"); break; }
    case Diatom::Type::String: { printf("string"); break; }
    case Diatom::Type::Bool:   { printf("bool");   break; }
    case Diatom::Type::Nil:    { printf("nil");    break; }
    case Diatom::Type::Table:  { printf("table");  break; }
  }
  printf("\n");
}


std::vector<std::string> get_type_list(std::vector<Token> ts) {
  return _DiatomSerialization::map<Token, std::string>(ts, [](Token t) {
    return t.type_string();
  });
}


void testDiatom() {
  p_file_header("Diatom.h");

  p_header("number type");
  Diatom n1((double) 5);
  Diatom n2(7.42);
  p_assert(n1.is_number());
  p_assert(n1.value__number == 5.0);
  p_assert(n2.value__number == 7.42);


  p_header("boolean type");
  Diatom b1(false);
  Diatom b2(true);
  p_assert(b1.is_bool());
  p_assert(b2.is_bool());
  p_assert(b1.value__bool == false);
  p_assert(b2.value__bool == true);


  p_header("string type");
  std::string str = "Hi, I'm a string";
  const char *cstr = "I'm a C string";
  Diatom s1(str);
  Diatom s2(cstr);
  p_assert(s1.is_string());
  p_assert(s2.is_string());
  p_assert(s1.value__string == str);
  p_assert(s2.value__string == cstr);


  p_header("nil type");
  Diatom nil1 = Diatom(Diatom::Type::Nil);
  Diatom nil2 = Diatom(Diatom::Type::Nil);
  p_assert(nil1.is_nil());


  p_header("tables");
  Diatom t1;
  p_assert(t1.is_table());
  p_assert(t1["monkeys"].is_nil());
  t1["custard"] = "lemons";
  t1["bananas"] = false;
  p_assert(t1.descendants.size() == 3);
  p_assert(t1["monkeys"].is_nil());
  p_assert(t1["custard"].is_string());
  p_assert(t1["bananas"].is_bool());
  t1["subtable"] = Diatom();
  t1["subtable"]["pears"];
  t1["subtable"]["oranges"] = 0.0;
  p_assert(t1["subtable"].is_table());
  p_assert(t1["subtable"]["pears"].is_nil());
  p_assert(t1["subtable"]["oranges"].is_number());


  p_header("table copying");
  Diatom scientists;
  scientists["pavlov"] = "Dogs";
  Diatom russians;
  russians["nikolai"] = "Lugansky";
  russians["mikhail"] = "Gorbachev";
  russians["scientists"] = scientists;
  Diatom r2 = russians;
  p_assert(russians.is_table());
  p_assert(r2.is_table());
  p_assert(r2.descendants.size() == 3);
  p_assert(r2["scientists"].is_table());
  p_assert(r2["scientists"].descendants.size() == 1);
  p_assert(r2["mikhail"].is_string());
  p_assert(r2["mikhail"].value__string == "Gorbachev");


  p_file_header("DiatomSerialization.h");
  p_header("float_format");
  auto str1 = _DiatomSerialization::float_format(11235);
  auto str2 = _DiatomSerialization::float_format(2983763.25);
  auto str3 = _DiatomSerialization::float_format(2.4);
  p_assert(str1 == "11235");
  p_assert(str2 == "2983763.25");
  p_assert(str3 == "2.39999999999999991");


  p_header("indent");
  std::string indent1 = _DiatomSerialization::indent(1);
  std::string indent2 = _DiatomSerialization::indent(2);
  p_assert(indent1 == "  ");
  p_assert(indent2 == "    ");

  p_header("filter");
  std::vector<std::string> filter_strings = {
    "hello",
    "there",
    "world",
  };
  auto filter_strings_result = _DiatomSerialization::filter(filter_strings, [](std::string s) {
    return s.find('o') != std::string::npos;
  });
  std::vector<std::string> filter_strings_exp = {
    "hello",
    "world",
  };
  p_assert(filter_strings_result == filter_strings_exp);


  p_header("map_property");
  std::vector<Token> mp_tokens = {
    { Token::Name },
    { Token::Property__String },
  };
  auto mp_types = _DiatomSerialization::map<Token, Token::Type>(mp_tokens, [](Token t) {
    return t.type;
  });
  std::vector<Token::Type> mp_types_exp{ Token::Name, Token::Property__String };
  p_assert(mp_types == mp_types_exp);


  p_header("contains");
  std::vector<std::string> contains_strings = {
    "monkeys",
    "lemurs",
    "penguins",
  };
  bool has_penguins = _DiatomSerialization::contains(contains_strings, "penguins");
  bool has_batman   = _DiatomSerialization::contains(contains_strings, "batman");
  p_assert(has_penguins == true);
  p_assert(has_batman == false);


  p_header("split");
  auto split_result        = _DiatomSerialization::split("hello there everyone.", ' ');
  auto split_result_empty  = _DiatomSerialization::split("", ' ');
  auto split_result_consec = _DiatomSerialization::split("   ", ' ');
  auto split_result_none   = _DiatomSerialization::split("hello", ' ');
  std::vector<std::string> split_result_exp{ "hello", "there", "everyone." };
  std::vector<std::string> split_result_consec_exp{ "", "", "" };
  std::vector<std::string> split_result_none_exp{ "hello" };
  p_assert(split_result == split_result_exp);
  p_assert(split_result_empty.size() == 0);
  p_assert(split_result_consec == split_result_consec_exp);
  p_assert(split_result_none == split_result_none_exp);


  p_header("diatom__serialize()");
  Diatom dsz1;
  dsz1["coati"] = 12.;
  dsz1["has_coati"] = true;
  dsz1["lemurs"] = "no";
  dsz1["has_lemurs"] = false;
  dsz1["birds"] = Diatom();
  dsz1["birds"]["penguins"] = 17.;
  dsz1["birds"]["ostriches"] = "five";
  Diatom dsz2 = "Muffins";
  auto s__dsz1 = diatom__serialize(dsz1);
  auto s__dsz2 = diatom__serialize(dsz2);
  const char *exp__dsz1 =
    "coati: 12\n"
    "has_coati: true\n"
    "lemurs: \"no\"\n"
    "has_lemurs: false\n"
    "birds:\n"
    "  penguins: 17\n"
    "  ostriches: \"five\"\n";
  const char *exp__dsz2 = "\"Muffins\"\n";
  p_assert(s__dsz1 == exp__dsz1);
  p_assert(s__dsz2 == exp__dsz2);


  p_header("token getters");
  auto texp_invalid = Token{ Token::Invalid };

  std::string tok_name = "a100_7F:";
  std::string tok_notname = "7b";
  std::string tok_booly = "true";
  auto t_name    = _DiatomSerialization::token__name(tok_name.begin(), tok_name);
  auto t_notname = _DiatomSerialization::token__name(tok_notname.begin(), tok_notname);
  auto t_booly   = _DiatomSerialization::token__name(tok_booly.begin(), tok_booly);
  auto texp_name = Token{ Token::Name, 0, "a100_7F" };
  p_assert(t_name == texp_name);
  p_assert(t_notname == texp_invalid);
  p_assert(t_booly == texp_invalid);

  std::string tok_string = "\"well here's a \\\"string\\\"\"";
  std::string tok_stringnl = "\"well here's a...\nstring\"";
  std::string tok_notstring = "29384";
  auto t_string    = _DiatomSerialization::token__str_property(tok_string.begin(), tok_string);
  auto t_stringnl  = _DiatomSerialization::token__str_property(tok_stringnl.begin(), tok_stringnl);
  auto t_notstring = _DiatomSerialization::token__str_property(tok_notstring.begin(), tok_notstring);
  auto texp_string = Token{ Token::Property__String, 0, "\"well here's a \\\"string\\\"\"" };
  p_assert(t_string == texp_string);
  p_assert(t_stringnl.type == Token::Error);
  p_assert(t_notstring == texp_invalid);

  std::string tok_number = "13.245";
  std::string tok_notnumber = "f39284";
  auto t_number    = _DiatomSerialization::token__number_property(tok_number.begin(), tok_number);
  auto t_notnumber = _DiatomSerialization::token__number_property(tok_notnumber.begin(), tok_notnumber);
  auto texp_number = Token{ Token::Property__Number, 13.245, "13.245" };
  p_assert(t_number == texp_number);
  p_assert(t_notnumber == texp_invalid);

  std::string tok_bool1 = "truexyz";
  std::string tok_bool2 = "falsexyz";
  std::string tok_bool3 = "xyztruefalse";
  auto t_bool1 = _DiatomSerialization::token__bool_property(tok_bool1.begin(), tok_bool1);
  auto t_bool2 = _DiatomSerialization::token__bool_property(tok_bool2.begin(), tok_bool2);
  auto t_bool3 = _DiatomSerialization::token__bool_property(tok_bool3.begin(), tok_bool3);
  auto texp_bool1 = Token{ Token::Property__Bool, 0, "true" };
  auto texp_bool2 = Token{ Token::Property__Bool, 0, "false" };
  p_assert(t_bool1 == texp_bool1);
  p_assert(t_bool2 == texp_bool2);
  p_assert(t_bool3 == texp_invalid);

  std::string tok_spaces  = "    x";
  std::string tok_tabs    = "    y";
  std::string tok_neither = "x  ";
  std::string tok_mixed   = "\t  z";
  auto t_spaces  = _DiatomSerialization::token__whitespace(tok_spaces.begin(), tok_spaces);
  auto t_tabs    = _DiatomSerialization::token__whitespace(tok_tabs.begin(), tok_tabs);
  auto t_neither = _DiatomSerialization::token__whitespace(tok_neither.begin(), tok_neither);
  auto t_mixed   = _DiatomSerialization::token__whitespace(tok_mixed.begin(), tok_mixed);
  auto texp_spaces = Token{ Token::Whitespace, 0, "    " };
  auto texp_tabs   = Token{ Token::Whitespace, 0, "    " };
  p_assert(t_spaces == texp_spaces);
  p_assert(t_tabs == texp_tabs);
  p_assert(t_neither == texp_invalid);
  p_assert(t_mixed.type == Token::Error);

  std::string tok_colon = ":";
  std::string tok_notcolon = "@";
  auto t_colon = _DiatomSerialization::token__colon(tok_colon.begin());
  auto t_notcolon = _DiatomSerialization::token__colon(tok_notcolon.begin());
  auto texp_colon = Token{ Token::Colon, 0, ":" };
  p_assert(t_colon == texp_colon);
  p_assert(t_notcolon == Token{ Token::Invalid });


  p_header("matching_tokens()");
  std::string input_name       = "hello there";
  std::string input_string     = "\"a string\"0.187";
  std::string input_number     = "39485.95\n";
  std::string input_bool       = "true\n";
  std::string input_whitespace = "   muffins";
  std::string input_colon      = ":\n";
  auto ts_name       = _DiatomSerialization::matching_tokens(input_name.begin(),       input_name);
  auto ts_string     = _DiatomSerialization::matching_tokens(input_string.begin(),     input_string);
  auto ts_number     = _DiatomSerialization::matching_tokens(input_number.begin(),     input_number);
  auto ts_bool       = _DiatomSerialization::matching_tokens(input_bool.begin(),       input_bool);
  auto ts_whitespace = _DiatomSerialization::matching_tokens(input_whitespace.begin(), input_whitespace);
  auto ts_colon      = _DiatomSerialization::matching_tokens(input_colon.begin(),      input_colon);
  auto ts_name_strings       = get_type_list(ts_name);
  auto ts_string_strings     = get_type_list(ts_string);
  auto ts_number_strings     = get_type_list(ts_number);
  auto ts_bool_strings       = get_type_list(ts_bool);
  auto ts_whitespace_strings = get_type_list(ts_whitespace);
  auto ts_colon_strings      = get_type_list(ts_colon);
  auto ts_name_exp       = std::vector<std::string>{ "Name" };
  auto ts_string_exp     = std::vector<std::string>{ "Property__String" };
  auto ts_number_exp     = std::vector<std::string>{ "Property__Number" };
  auto ts_bool_exp       = std::vector<std::string>{ "Property__Bool" };
  auto ts_whitespace_exp = std::vector<std::string>{ "Whitespace" };
  auto ts_colon_exp      = std::vector<std::string>{ "Colon" };
  p_assert(ts_name_strings       == ts_name_exp);
  p_assert(ts_string_strings     == ts_string_exp);
  p_assert(ts_number_strings     == ts_number_exp);
  p_assert(ts_bool_strings       == ts_bool_exp);
  p_assert(ts_whitespace_strings == ts_whitespace_exp);
  p_assert(ts_colon_strings      == ts_colon_exp);


  p_header("strip_nonleading_whitespace");
  auto stripws_result = _DiatomSerialization::strip_nonleading_whitespace(std::vector<Token>{
    { Token::Type::Whitespace },
    { Token::Type::Property__Bool },
    { Token::Type::Whitespace },
    { Token::Type::Colon },
    { Token::Type::Whitespace},
  });
  std::vector<Token> stripws_exp = {
    { Token::Type::Whitespace },
    { Token::Type::Property__Bool },
    { Token::Type::Colon },
  };
  p_assert(stripws_result == stripws_exp);


  p_header("find_inconsistent_whitespace");
  std::vector<std::vector<Token>> ws_consistent_sp = {
    {{ Token::Name, 0, "n" }},
    {{ Token::Whitespace, 0, "  " }},
  };
  std::vector<std::vector<Token>> ws_first_line = {
    {{ Token::Whitespace, 0, "  " }},
    {{ Token::Name, 0, "n" }},
  };
  std::vector<std::vector<Token>> ws_inconsistent_sameline = {
    {{ Token::Name, 0, "n" }},
    {{ Token::Whitespace, 0, "  \t" }},
    {{ Token::Name, 0, "n" }},
    {{ Token::Whitespace, 0, "  " }},
  };
  std::vector<std::vector<Token>> ws_inconsistent_across_lines = {
    {{ Token::Name, 0, "n" }},
    {{ Token::Whitespace, 0, "\t" }},
    {{ Token::Whitespace, 0, "  " }},
  };
  std::vector<std::vector<Token>> ws_odd_space_indent = {
    {{ Token::Name, 0, "n" }},
    {{ Token::Whitespace, 0, "   " }},
  };
  std::vector<std::vector<Token>> ws_inappropriate_indent1 = {
    {{ Token::Name, 0, "n" }, { Token::Colon, 0, ":" }, { Token::Property__Number, 3, "3" }},
    {{ Token::Whitespace, 0, "  " }},
  }; // An indent is inappropriate where it increases following a nontable property line
  std::vector<std::vector<Token>> ws_inappropriate_indent2 = {
    {{ Token::Name, 0, "n" }},
    {{ Token::Whitespace, 0, "\t\t" }},
  }; // ...or when it increases following a table line by more than 1.
  size_t ws_consistent_sp_result             = _DiatomSerialization::find_inconsistent_whitespace(ws_consistent_sp);
  size_t ws_first_line_result                = _DiatomSerialization::find_inconsistent_whitespace(ws_first_line);
  size_t ws_inconsistent_sameline_result     = _DiatomSerialization::find_inconsistent_whitespace(ws_inconsistent_sameline);
  size_t ws_inconsistent_across_lines_result = _DiatomSerialization::find_inconsistent_whitespace(ws_inconsistent_across_lines);
  size_t ws_odd_space_indent_result          = _DiatomSerialization::find_inconsistent_whitespace(ws_odd_space_indent);
  size_t ws_inappropriate_indent1_result     = _DiatomSerialization::find_inconsistent_whitespace(ws_inappropriate_indent1);
  size_t ws_inappropriate_indent2_result     = _DiatomSerialization::find_inconsistent_whitespace(ws_inappropriate_indent2);
  p_assert(ws_consistent_sp_result == -1);
  p_assert(ws_first_line_result == 0);
  p_assert(ws_inconsistent_sameline_result == 1);
  p_assert(ws_inconsistent_across_lines_result == 2);
  p_assert(ws_odd_space_indent_result == 1);
  p_assert(ws_inappropriate_indent1_result == 1);
  p_assert(ws_inappropriate_indent2_result == 1);


  p_header("line_is_valid()");
  std::vector<Token> line_withprop = {
    { Token::Name },
    { Token::Colon },
    { Token::Property__Bool },
  };
  bool result__line_withprop = _DiatomSerialization::line_is_valid(line_withprop);
  p_assert(result__line_withprop == true);

  std::vector<Token> line_withoutprop = {
    { Token::Name },
    { Token::Colon },
  };
  bool result__line_withoutprop = _DiatomSerialization::line_is_valid(line_withoutprop);
  p_assert(result__line_withoutprop == true);

  std::vector<Token> line_withprop_extra = {
    { Token::Name },
    { Token::Colon },
    { Token::Property__Bool },
    { Token::Colon },
  };
  bool result__line_withprop_extra = _DiatomSerialization::line_is_valid(line_withprop_extra);
  p_assert(result__line_withprop_extra == false);

  std::vector<Token> line_garbage = {
    { Token::Colon },
    { Token::Whitespace },
    { Token::Name },
    { Token::Property__String },
  };
  bool result__line_garbage = _DiatomSerialization::line_is_valid(line_garbage);
  p_assert(result__line_garbage == false);


  p_header("tokenize()");
  std::string tkz_input = "    a_prop: 7.413";
  auto tkz_result = _DiatomSerialization::tokenize(tkz_input);
  std::vector<Token>tkz_exp{
    { Token::Whitespace,       0, "    "   },
    { Token::Name,             0, "a_prop" },
    { Token::Colon,            0, ":"      },
    { Token::Whitespace,       0, " "      },
    { Token::Property__Number, 7.413, "7.413"  },
  };
  p_assert(tkz_result == tkz_exp);
  std::string tkz_input_fail = "   my@prop: 6";

  auto tkz_result_fail = _DiatomSerialization::tokenize(tkz_input_fail);
  p_assert(tkz_result_fail.size() == 1);
  p_assert(tkz_result_fail[0].type == Token::Error);


  p_header("tokens_to_line()");
  auto ttl_result = _DiatomSerialization::tokens_to_line({
    { Token::Whitespace, 0, "  " },
    { Token::Name, 0, "my_prop" },
    { Token::Colon, 0, ":" },
    { Token::Property__String, 0, "\"my_val\"" },
  });
  _DiatomSerialization::Line ttl_exp{
    1,
    {Token::Name, 0, "my_prop"},
    {Token::Property__String, 0, "\"my_val\""},
    {Token::Invalid},
    {Token::Invalid},
  };
  p_assert(ttl_result == ttl_exp);


  p_header("unserialize()");
  auto unsz_result = diatom__unserialize(
    "lemurs: 5\n"
    "birds:\n"
    "  blue_tits: \"14\"\n"
    "  aquatic:\n"
    "    penguins: 10\n"
    "  crows: false\n"
  );
  auto unsz_fail_result = diatom__unserialize("muffins: @7\n");
  DiatomParseResult unsz_fail_result_exp{
    false,
    "Unexpected input at line 1",
  };
  Diatom d = unsz_result.d;
  p_assert(unsz_result.success == true);
  p_assert(d.descendants.size() == 2);
  p_assert(d["lemurs"].is_number());
  p_assert(d["lemurs"].value__number == 5);
  p_assert(d["birds"].is_table());
  p_assert(d["birds"].descendants.size() == 3);
  p_assert(d["birds"]["blue_tits"].is_string());
  p_assert(d["birds"]["blue_tits"].value__string == "\"14\"");
  p_assert(d["birds"]["crows"].is_bool());
  p_assert(d["birds"]["crows"].value__bool == false);
  p_assert(d["birds"]["aquatic"].is_table());
  p_assert(d["birds"]["aquatic"]["penguins"].is_number());
  p_assert(d["birds"]["aquatic"]["penguins"].value__number == 10);
  p_assert(unsz_fail_result == unsz_fail_result_exp);
}


int main() {
  testDiatom();
  return 0;
}

