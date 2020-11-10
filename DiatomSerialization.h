//
// DiatomSerialization.h
//
// - A .diatom file:
//
//    various_items:
//      a_number: 7
//      a_string: "Hello, world"
//      a_bool: true
//    animals:
//      birds:
//        penguins: 7
//
// Format:
//  - values are tables, strings, numbers, or booleans
//  - names must begin with a letter, contain only alphanumeric + underscore.
//  - indenting: 2 spaces or 1 tab
//
// MIT licensed - http://opensource.org/licenses/MIT
// -- BH 2012
//

#ifndef __DiatomSerialization_h
#define __DiatomSerialization_h

#include "Diatom.h"
#include <string>
#include <cstdio>


// Interface
// -----------------------------

struct DiatomParseResult {
  bool success;
  std::string error_string;
  Diatom d;

  bool operator==(DiatomParseResult &r) {
    return success == r.success && error_string == r.error_string;
  }
};

static std::string diatom__serialize(Diatom &d);
static DiatomParseResult diatom__unserialize(const std::string &);



// Implementation
// -----------------------------

struct _DiatomSerialization {

  // Helpers
  // -----------------------------

  static std::string float_format(double x) {
    std::string s;
    s.insert(0, 100, '0');
    sprintf(&s[0], "%.17f", x);

    std::string::iterator it__last_nonzero;
    for (auto it = s.begin(); it < s.end(); ++it) {
      if (*it != '0' && *it != '\0') {
        it__last_nonzero = it;
      }
    }

    return std::string(
      s.begin(),
      it__last_nonzero + (*it__last_nonzero == '.' ? 0 : 1)
    );

    // Note: within a nonzero range of the real numbers the proportion that are
    // perfectly representable as a double is zero.
    // So some numbers will serialize as expected: eg integers, multiples of 0.25.
    // Others will include the floating point error, e.g. 2.4 -> 2.39999999999999991
  }

  static bool has_both_tabs_and_spaces(const std::string &s) {
    bool contains_space = s.find(' ') != std::string::npos;
    bool contains_tab   = s.find('\t') != std::string::npos;
    return contains_space && contains_tab;
  }

  static std::string indent(size_t n) {
    std::string s;
    s.insert(0, n*2, ' ');
    return s;
  }

  template <class Vec, class F>
  static Vec filter(const Vec &v, F f) {
    Vec out;
    for (auto &i : v) {
      if (f(i)) {
        out.push_back(i);
      }
    }
    return out;
  }

  template <class VectorItem, class ReturnItem, class F>
  static std::vector<ReturnItem> map(const std::vector<VectorItem> &v, F f) {
    std::vector<ReturnItem> out;
    for (auto &i : v) {
      out.push_back(f(i));
    }
    return out;
  }

  template <class Vec, class Item>
  static bool contains(const Vec &v, Item x) {
    for (auto &i : v) {
      if (i == x) {
        return true;
      }
    }
    return false;
  }

  static std::vector<std::string> split(std::string s, char c) {
    if (s.length() == 0) {
      return { };
    }

    std::vector<std::string::iterator> i_splits;
    for (auto i = s.begin(); i < s.end(); ++i) {
      if (*i == c) {
        i_splits.push_back(i);
      }
    }

    if (i_splits.size() == 0) {
      return { s };
    }

    std::vector<std::string> out;
    for (size_t i=0; i < i_splits.size(); ++i) {
      auto terminal_newline = i_splits[i];
      auto prev_newline = i == 0 ? s.begin() : i_splits[i - 1];
      out.push_back(std::string{ prev_newline + (i == 0 ? 0 : 1), terminal_newline });
    }
    if (i_splits.back() != s.end() - 1) {
      out.push_back(std::string{ i_splits.back() + 1, s.end() });
    }
    return out;
  }

  static bool is_whitespace(char c) { return c == ' ' || c == '\t'; }
  static bool is_numeric(char c) { return c >= '0' && c <= '9'; }
  static bool is_az(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
  static bool is_alphanumeric(char c) { return is_az(c) || is_numeric(c); }
  static bool is_alphanumeric_or_underscore(char c) { return is_alphanumeric(c) || c == '_'; }


  // Serialize
  // -----------------------------

  static std::string serialize(Diatom &d, size_t indentation = 0, bool prefix_space = false) {
    std::string s;
    std::string key_value_space = std::string(prefix_space ? " " : "");

    if (d.is_table()) {
      if (indentation > 0) {
        s += "\n";
      }
      d.each([&s, indentation](std::string &key, Diatom &d) -> void {
        if (d.is_empty()) {
          return;
        }
        s += indent(indentation);
        s += key + std::string(":");
        s += serialize(d, indentation + 1, true);
      });
    }
    else if (d.is_number()) { s += key_value_space + float_format(d.number_value); }
    else if (d.is_string()) { s += key_value_space + std::string("\"") + d.string_value + "\""; }
    else if (d.is_bool())   { s += key_value_space + (d.bool_value ? "true" : "false"); }
    else if (d.is_empty())  { };

    if (!d.is_table()) {
      s += "\n";
    }
    return s;
  }


  // Token type for unserialization
  // -----------------------------

  struct Token {
    enum Type {
      EndOfString,
      Name,
      Property__String,
      Property__Number,
      Property__Bool,
      Whitespace,
      Colon,
      Comma,
      Invalid,
      Error
    };

    Type        type;
    double      n;
    std::string s;
    std::string error_str;

    bool operator==(const Token &t) const {
      return (
        type == t.type  &&
        t.n - n < 0.001 &&
        s == t.s        &&
        error_str == t.error_str
      );
    }

    std::string type_string() {
      return __type_string(type);
    }

    static std::string __type_string(Token::Type type) {
      return (
        type == EndOfString      ? "EndOfString"      :
        type == Name             ? "Name"             :
        type == Property__String ? "Property__String" :
        type == Property__Number ? "Property__Number" :
        type == Property__Bool   ? "Property__Bool"   :
        type == Whitespace       ? "Whitespace"       :
        type == Colon            ? "Colon"            :
        type == Comma            ? "Comma"            :
        type == Invalid          ? "Invalid"          :
        type == Error            ? "Error"            :
        "Unknown"
      );
    }
  };

  typedef std::vector<Token> TokenVector;
  typedef std::vector<Token::Type> TokenTypeVector;


  // Tokenisation functions
  // -----------------------------

  static Token token__name(std::string::iterator it, std::string &s) {
    if (!is_az(*it)) {
      return Token{ Token::Invalid };
    }

    std::string out;
    for (; it < s.end() && is_alphanumeric_or_underscore(*it); ++it) {
      out += *it;
    }
    if (out == "true" || out == "false") {
      return Token{ Token::Invalid };
    }
    return Token{ Token::Name, 0, out };
  }

  static Token token__str_property(std::string::iterator it, std::string &s) {
    if (*it != '"') {
      return Token{ Token::Invalid };
    }

    std::string out = "\"";
    for (it += 1; it < s.end(); ++it) {
      char c = *it;
      bool is_escaped = it != s.begin() && *(it-1) == '\\';
      if (c == '\n' || c == '\r') {
        return Token{ Token::Error, 0, "", "Unexpected newline in string literal" };
      }
      out += c;
      if (c == '"' && !is_escaped) {
        break;
      }
    }
    return Token{ Token::Property__String, 0, out };
  }

  static Token token__number_property(std::string::iterator it, std::string &s) {
    if (!(is_numeric(*it) || *it == '.' || *it == '-')) {
      return Token{ Token::Invalid };
    }
    try {
      size_t i_after;
      float n = std::stof(std::string(it, s.end()), &i_after);
      return Token{ Token::Property__Number, n, std::string(it, it + i_after) };
    }
    catch(...) {
      return Token{ Token::Invalid };
    }
  }

  static Token token__bool_property(std::string::iterator it, std::string &s) {
    auto substr__true = s.substr(it - s.begin(), 4);
    auto substr__false = s.substr(it - s.begin(), 5);

    if (substr__true == "true") {
      return Token{ Token::Property__Bool, 0, "true" };
    }
    else if (substr__false == "false") {
      return Token{ Token::Property__Bool, 0, "false" };
    }
    return Token{ Token::Invalid };
  }

  static Token token__whitespace(std::string::iterator it, std::string &s) {
    std::string whitespace;
    for (; it < s.end() && is_whitespace(*it); ++it) {
      whitespace += *it;
    }

    if (whitespace.length() == 0) {
      return Token{ Token::Invalid };
    }
    if (has_both_tabs_and_spaces(whitespace)) {
      return Token{ Token::Error, 0, "", "Input contains both tabs and spaces" };
    }
    return Token{ Token::Whitespace, 0, whitespace };
  }

  static Token token__colon(std::string::iterator it) {
    return *it == ':' ?
      Token{ Token::Colon, 0, ":" } :
      Token{ Token::Invalid };
  }


  // Parsing
  // -----------------------------

  struct Line {
    size_t indent;

    Token name;
    Token prop_str;
    Token prop_num;
    Token prop_bool;

    Diatom d;

    bool operator==(const Line &l) {
      return (
        indent == l.indent &&
        name == l.name &&
        prop_str  == l.prop_str &&
        prop_num  == l.prop_num &&
        prop_bool == l.prop_bool
      );
    }

    bool is_table() {
      return (
        prop_str.type  == Token::Invalid &&
        prop_num.type  == Token::Invalid &&
        prop_bool.type == Token::Invalid
      );
    }
  };

  static bool line_is_valid(TokenVector tokens) {
    // NB: non-leading optional whitespace items and empty lines must have been stripped
    if (tokens.size() == 0) {
      return false;
    }
    auto types = map<Token, Token::Type>(tokens, [](Token t)  {
      return t.type;
    });
    if (types[0] == Token::Whitespace) {
      types = std::vector<Token::Type> (types.begin() + 1, types.end());
    }

    // Lines without properties (ie tables): Name, Colon
    if (types == std::vector<Token::Type>{
      Token::Name,
      Token::Colon,
    }) {
      return true;
    }

    // Lines with properties: Name, Colon, Prop
    return (
      types.size() == 3 &&
      TokenTypeVector(types.begin(), types.begin() + 2) == TokenTypeVector{
        Token::Name,
        Token::Colon,
      } &&
      contains(TokenTypeVector{
        Token::Property__Number,
        Token::Property__String,
        Token::Property__Bool,
      }, types[2])
    );
  }

  static TokenVector matching_tokens(std::string::iterator it, std::string &s) {
    TokenVector tokens = {
      token__name(it, s),
      token__str_property(it, s),
      token__number_property(it, s),
      token__bool_property(it, s),
      token__whitespace(it, s),
      token__colon(it),
    };
    TokenVector valid_tokens = filter(tokens, [](Token t) {
      return t.type != Token::Invalid;
    });
    return valid_tokens;
  }

  static TokenVector strip_nonleading_whitespace(const TokenVector &ts) {
    if (ts.size() == 0) {
      return ts;
    }
    TokenVector out{ ts[0] };
    TokenVector allbutfirst{ts.begin() + 1, ts.end()};
    allbutfirst = filter(allbutfirst, [](Token t) {
      return t.type != Token::Whitespace;
    });
    out.insert(out.end(), allbutfirst.begin(), allbutfirst.end());
    return out;
  }

  static size_t calculate_indent(TokenVector line) {
    size_t indent = 0;
    if (line[0].type == Token::Whitespace) {
      if (line[0].s[0] == ' ') {
        indent = line[0].s.length() / 2;   // 2 spaces per indent
      }
      else {
        indent = line[0].s.length();       // 1 tab per indent
      }
    }
    return indent;
  }

  static size_t find_inconsistent_whitespace(const std::vector<TokenVector> lines) {
    int ws_type = 0;  // 0 for not yet discovered, 1 for tabs, 2 for spaces
    if (lines.size() == 0) {
      return -1;
    }

    // The first line cannot start with whitespace
    if (lines[0][0].type == Token::Whitespace) {
      return 0;
    }

    for (auto i = lines.begin(); i < lines.end(); ++i) {
      Token t = (*i)[0];
      if (t.type == Token::Whitespace) {
        std::string s = t.s;
        if (has_both_tabs_and_spaces(s)) {
          return i - lines.begin();
        }

        // Establish whitespace type
        if (ws_type == 0) {
          ws_type = s[0] == '\t' ? 1 : 2;
        }
        // Check uses a consistent whitespace type
        else {
          if ((s[0] == '\t' && ws_type == 2) || (s[0] == ' ' && ws_type == 1)) {
            return i - lines.begin();
          }
        }

        // Must not be odd numbers of spaces
        if (ws_type == 2 && s.length()%2 != 0) {
          return i - lines.begin();
        }

        if (i != lines.begin()) {
          TokenVector lprev = *(i - 1);
          bool prev_was_property_line = filter(lprev, [](Token t) {
            return (
              t.type == Token::Property__String ||
              t.type == Token::Property__Number ||
              t.type == Token::Property__Bool
            );
          }).size() > 0;
          int indent_change = int(calculate_indent(*i)) - int(calculate_indent(lprev));

          // Indent must not increase following a property line
          if (prev_was_property_line && indent_change > 0) {
            return i - lines.begin();
          }

          // Indent must not increase by more than 1 level
          if (indent_change > 1) {
            return i - lines.begin();
          }
        }
      }
    }
    return -1;
  }

  static TokenVector tokenize(std::string &s) {
    auto it = s.begin();
    TokenVector out;

    while (it < s.end()) {
      auto ts = matching_tokens(it, s);
      if (ts.size() == 0 || (ts.size() == 1 && ts[0].type == Token::Error)) {
        return { { Token::Error, 0, "", "Invalid input" } };
      }
      // As per Modern Compiler Implementation, the chosen token is the longest
      // matching one
      size_t longest_token_i = 0;
      size_t longest_token_length = 0;
      for (size_t i=0; i < ts.size(); ++i) {
        auto t = ts[i];
        if (t.s.length() > longest_token_length) {
          longest_token_i = i;
          longest_token_length = t.s.length();
        }
      }
      Token longest = ts[longest_token_i];
      out.push_back(longest);
      it += longest.s.length();
    }

    return out;
  }

  static Line tokens_to_line(TokenVector ts) {
    Line line{
      0,
      Token{Token::Invalid},
      Token{Token::Invalid},
      Token{Token::Invalid},
      Token{Token::Invalid},
    };

    line.indent = calculate_indent(ts);
    if (ts[0].type == Token::Whitespace) {
      ts = TokenVector{ ts.begin() + 1, ts.end() };
    }

    line.name = ts[0];

    if (ts.size() == 2) {
      line.d = Diatom();
    }
    else {
      Token prop = ts[2];
      if (prop.type == Token::Property__String)      { line.prop_str = prop; }
      else if (prop.type == Token::Property__Number) { line.prop_num = prop; }
      else if (prop.type == Token::Property__Bool)   { line.prop_bool = prop; }
    }

    return line;
  }

  static Diatom line_to_single_diatom(Line l) {
    if (l.prop_str.type != Token::Invalid) {
      const std::string &s = l.prop_str.s;
      return s.length() > 0 ? std::string(s.begin() + 1, s.end() - 1) : s;
    }
    else if (l.prop_num.type != Token::Invalid) { return l.prop_num.n; }
    else if (l.prop_bool.type != Token::Invalid) {
      return l.prop_bool.s == "true" ? true : false;
    }
    return Diatom();
  }


  // Unserialization
  // -----------------------------

  static DiatomParseResult unserialize(std::string s) {
    while (s.back() == '\n') {
      s.pop_back();
    }
    size_t i_last_leading_newline = -1;
    for (auto i = s.begin(); *i == '\n' && i < s.end(); ++i) {
      ++i_last_leading_newline;
    }
    if (i_last_leading_newline != -1) {
      s = std::string(s.begin() + i_last_leading_newline + 1, s.end());
    }

    auto lines_str = split(s, '\n');
    auto lines_tok = map<std::string, TokenVector>(lines_str, [](std::string l) {
      return tokenize(l);
    });


    // Look for input errors
    for (auto i = lines_tok.begin(); i < lines_tok.end(); ++i) {
      auto v = *i;
      if (v.size() == 1 && v[0].type == Token::Error) {
        return {
          false,
          std::string("Unexpected input at line ") + std::to_string(i - lines_tok.begin() + 1),
        };
      }
    }

    // Strip non-leading whitespace
    lines_tok = map<TokenVector, TokenVector>(lines_tok, [](TokenVector ts) {
      return strip_nonleading_whitespace(ts);
    });

    // Check lines are valid
    for (auto i = lines_tok.begin(); i < lines_tok.end(); ++i) {
      if (!line_is_valid(*i)) {
        return {
          false,
          std::string("Invalid line structure at line ") + std::to_string(i - lines_tok.begin() + 1),
        };
      }
    }

    // Check whitespace is consistent
    size_t i_inconsistent_whitespace = find_inconsistent_whitespace(lines_tok);
    if (i_inconsistent_whitespace != -1) {
      return {
        false,
        std::string("Inconsistent whitespace found at line ") + std::to_string(i_inconsistent_whitespace + 1),
      };
    }

    // Convert to lines
    auto lines = map<TokenVector, Line>(lines_tok, [](TokenVector ts) {
      return tokens_to_line(ts);
    });

    // Compose diatoms
    Diatom top;
    for (size_t i = lines.size() - 1; i != -1; --i) {
      Line &l = lines[i];
      if (l.d.table_entries.size() == 0) {
        l.d = line_to_single_diatom(l);
      }

      Diatom *parent = NULL;
      size_t j;
      for (j = i - 1; j != -1; --j) {
        Line &ancestor_line = lines[j];
        if (l.indent > 0 && l.indent - ancestor_line.indent == 1) {
          parent = &ancestor_line.d;
          break;
        }
      }

      (*(parent ? parent : &top))[l.name.s] = l.d;
    }
    top.recurse([](std::string key, Diatom &d) {
      if (d.is_table()) {
        std::reverse(d.table_entries.begin(), d.table_entries.end());
      }
    });

    return { true, "", top };
  }
};


// Interface implementations
// -----------------------------

std::string diatom__serialize(Diatom &d) {
  return _DiatomSerialization::serialize(d);
}

DiatomParseResult diatom__unserialize(const std::string &s) {
  return _DiatomSerialization::unserialize(s);
}

#endif

