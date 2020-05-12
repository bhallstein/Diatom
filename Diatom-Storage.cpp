/*
 * Diatom-Storage.cpp
 *
 * Copyright (c) 2015 - Ben Hallstein - ben.am
 * Published under the MIT license - http://opensource.org/licenses/MIT
 *
 */

#include "Diatom-Storage.h"
#include <unistd.h>
#include <vector>

using Str = std::string;

namespace DSHelpers {
	Str __floatFmt(double x) {
		char s[100];
		for (int i=0; i < 100; ++i) s[i] = '0';
		sprintf(s, "%.17f", x);

		int i=99;
		for (; i >= 0 && s[i] != '.' && (s[i] == '0' || s[i] == '\0'); --i) ;
		if (s[i] == '.') s[i] = '\0';
		else             s[i+1] = '\0';
	
		return Str(s);
	}
}

#define __INDENT(s, n) do { for (int i=0; i < n; ++i) s += "  "; } while (false)
#define CH_WHITESPACE(c) (c == ' ' || c == '\t')
#define CH_NUMERIC(c)   ((c >= '0' && c <= '9') || c == '.')
#define CH_ALPHA(c)     ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define CH_ALPHANUM_(c)  (CH_ALPHA(c) || CH_NUMERIC(c) || c == '_')


Str __diatomToString(Diatom &d, int indentLevel = 0) {
	Str s;
	if (d.isTable()) {
		int n_desc = 0;
		for (auto &i : d.descendants()) {
			if (i.second.isNil()) continue;
			s += "\n";
			__INDENT(s, indentLevel);
			s += i.first + Str(":");
			s += __diatomToString(i.second, indentLevel+1);
			++n_desc;
		}
	}
	else if (d.isNumber()) s += Str(" ") + DSHelpers::__floatFmt(d.number_value());
	else if (d.isString()) s += Str(" ") + Str("\"") + d.str_value() + "\"";
	else if (d.isBool())   s += Str(" ") + (d.bool_value() ? "true" : "false");
	
	return s;
}


#pragma mark - diatomToString

Str diatomToString(Diatom &d) {
    if (d.isNil()) return "";
    return __diatomToString(d) + "\n";
}

Str diatomToString(Diatom &d, const Str &name) {
	if (d.isNil()) return "";
	Diatom d_wrapper;
	d_wrapper[name] = d;
	return diatomToString(d_wrapper);
}


#pragma mark - diatomFromString

Diatom diatomFromString(const Str &str) {
	// Tasks:
	//  - check format is valid
	//     - indenting whitespace is as expected (get from first indented property)
	//     - names valid, and followed with /:[ ]*/
	//     - properties are valid - strings or numbers or true/false
	//     - comments begin with # (except inside a string) and everything after them is ignored until /n
	
	// General idea:
	//  - repeatedly parse lines
	//  - identify what is going on:
	//      - same indent level:   adding a property
	//      - lower indent level:  end current N tables up the stack
	//      - no property value on line: begin a new sub-table
	
	struct Token {
		enum Type {
			EndOfString,
			EndOfLine,
			Name,
			StringProperty,
			NumberProperty,
			BoolProperty,
			Whitespace,
			Colon,
			Comma,
			Error
		} type;
		Str s = "";
		double n;
		
		Str error_str = "";
		
		Token(Type t) : type(t) { }
		Token(Type t, const Str &_s) : type(t), s(_s) { }
		static Token mkError(const Str &e) {
			Token t(Error);
			t.error_str = e;
			return t;
		}
		static Str typeToStr(Type t) {
			return (
				t == EndOfString    ? "EndOfString"    :
				t == EndOfLine      ? "EndOfLine"      :
				t == Name           ? "Name"           :
				t == StringProperty ? "StringProperty" :
				t == NumberProperty ? "NumberProperty" :
				t == BoolProperty   ? "BoolProperty"   :
				t == Whitespace     ? "Whitespace"     :
				t == Colon          ? "Colon"          :
				t == Comma          ? "Comma"          : "Unknown"
			);
		}
	};
 	
	auto getToken = [&](Str &input) {
		int i, n = (int)input.size();
		if (n == 0) return Token(Token::EndOfString);
		
		char c = input[0];
		
		// EOL
		if (c == '\n') {
			if (n == 1) input = "";
			else        input = Str(input.begin()+1, input.end());
			return Token(Token::EndOfLine);
		}
		if (c == '#') {
			for (i=0; i < n; ++i)
				if (input[i] == '\n') break;
			if (i == n) return Token(Token::EndOfString);
			if (i == n-1) input = "";
			else          input = Str(input.begin()+i+1, input.end());
			return Token(Token::EndOfLine);
		}
		
		// String literal
		if (c == '"' || c == '\'') {
			if (input.size() == 1) return Token::mkError("unexpected end of string");
			for (i=1; i < n; ++i) {
				if (input[i] == '\n') return Token::mkError("unexpected newline");
				else if (input[i] == c){
					if (input[i-1] == '\\') {
						input = Str(input.begin(), input.begin()+i-1) + Str(input.begin()+i, input.end());
						i -= 1;
					}
					else {
						Str x = Str(input.begin()+1, input.begin()+i);
						if (i == n-1) input = "";
						else          input = Str(input.begin()+i+1, input.end());
						return Token(Token::StringProperty, x);
					}
				}
			}
			return Token::mkError("unexpected end of string");
		}
		
		// Number literal
		if (CH_NUMERIC(c) || c == '-') {
			Str num_str;
			num_str += c;
			if (n > 1)
				for (i=1; i < n && CH_NUMERIC(input[i]); ++i)
					num_str += input[i];
			if (i == n-1) input = "";
			else          input = Str(input.begin()+i, input.end());
			Token t(Token::NumberProperty);
			sscanf(num_str.c_str(), "%lf", &t.n);
			return t;
		}
		
		// Whitespace
		if (CH_WHITESPACE(c)) {
			for (i=0; i < n; ++i)
				if (!CH_WHITESPACE(input[i])) break;
			Str s = Str(input.begin(), input.begin()+i);
			if (i == n-1) input = "";
			else          input = Str(input.begin()+i, input.end());
			return Token(Token::Whitespace, s);
		}
		
		// Names, and boolean properties
		if (CH_ALPHA(c)) {
			for (i=0 ; i < n; ++i)
				if (!CH_ALPHANUM_(input[i])) break;
			Str s(input.begin(), input.begin()+i);
			if (i >= n-1) input = "";
			else          input = Str(input.begin()+i, input.end());
			if (s == Str("true") || s == Str("false"))
				return Token(Token::BoolProperty, s);
			
			return Token(Token::Name, s);
		}
		
		// Colon
		if (c == ':') {
			if (n == 1) input = "";
			else        input = Str(input.begin()+1, input.end());
			return Token(Token::Colon);
		}
		
		// Comma
		if (c == ',') {
			if (n == 1) input = "";
			else        input = Str(input.begin()+1, input.end());
			return Token(Token::Comma);
		}
		
		return Token::mkError("parse error");
	};
	
	// Token flow (per line):
	//   [ws] -> name -> [ws] -> colon -> [ws] -> property -> [ws] -> newline
	//                                        \-> newline
	
	struct State {
		Diatom d;
		std::vector<Diatom*> stack = { &d };
		
		Str input;

		enum Position {
			StartLine = 0,
			PropName,
			Colon,
			Property,
			PostProperty
		};
		Position pos = StartLine;
		
		Str pname;
		
		bool indentDiscovered = false;
		enum IndentType { IndentSpace, IndentTab } indentType;
		int indentNum = 0;
		
		int currentIndent = 0;
		int newIndent = 0;
		int newlines = 0;
		
		State(const Str &_input) : input(_input) { }
	} state(str);

	// StateErr()
	{
		#define StateErr(err_tok, line, exp_tok) \
		do {                                                                            \
			printf(                                                                     \
				"Diatom - parse error: unexpected %s token on line %d (expected %s)\n", \
				Token::typeToStr(err_tok).c_str(),                                      \
				state.newlines+1,                                                       \
				exp_tok                                                                 \
			);                                                                          \
			return Diatom::NilObject();                                                 \
		} while (0)
		#define StateErr_Ws(line, ind_num, type) \
		do {                                                                                        \
			printf(                                                                                 \
				"Diatom - parse error: invalid whitespace on line %d (using convention %d %s%s)\n", \
				line,                                                                               \
				ind_num,                                                                            \
				type == State::IndentSpace ? "space" : "tab",                                       \
				ind_num == 1 ? "" : "s"                                                             \
			);                                                                                      \
			return Diatom::NilObject();                                                             \
		} while (0)
	}
	
	while (1) {
		Token t = getToken(state.input);
		
		if (t.type == Token::Error) {
			printf(
				"Diatom - parse error: unexpected %s token on line %d\n",
				Token::typeToStr(t.type).c_str(),
				state.newlines+1
			);
			break;
		}
		
		if (t.type == Token::EndOfLine) {
			++state.newlines;
			state.newIndent = 0;
		}
		
		if (t.type == Token::EndOfString) {
			if (state.pos == State::StartLine || state.pos >= State::Property)
				break;
			StateErr(t.type, newlines+1, "EndOfLine or Whitespace");
		}
		
		if (t.type == Token::Whitespace && state.pos != State::StartLine) continue;
			// Ignore WS except for indenting
		
		if (t.type == Token::Comma && state.pos != State::PostProperty)
			StateErr(t.type, state.newlines+1, "EndOfLine or EndOfString");
		
		if (state.pos == State::StartLine) {
			if (t.type == Token::Whitespace) {
				// Discover indent
				if (!state.indentDiscovered) {
					char c = t.s[0];
					bool indent_type_consistent = true;
					for (auto &i : t.s) if (i != c) indent_type_consistent = false;
					if (!indent_type_consistent) {
						printf("Diatom - parse error: mix of tabs & spaces in first indent on line %d\n", state.newlines+1);
						return Diatom::NilObject();
					}
					state.indentType       = (c == ' ' ? State::IndentSpace : State::IndentTab);
					state.indentNum        = (int) t.s.size();
					state.indentDiscovered = true;
				}
				
				// Get indent quantity (error if invalid)
				int n=0;
				{
					char c_exp = (state.indentType == State::IndentSpace ? ' ' : '\t');
					for (auto &c : t.s) {
						if (c != c_exp)
							StateErr_Ws(state.newlines+1, state.indentNum, state.indentType);
						++n;
					}
					if (n%state.indentNum != 0) {
						StateErr_Ws(state.newlines+1, state.indentNum, state.indentType);
					}
				}
				state.newIndent = n / state.indentNum;
				
				// If indent is higher than expected, error
				if (state.newIndent > state.currentIndent)
					StateErr_Ws(state.newlines+1, state.indentNum, state.indentType);
			
				// If indent has decreased, pop table stack N times
				if (state.newIndent < state.currentIndent) {
					int N = state.currentIndent - state.newIndent;
					for (int i=0; i < N; ++i)
						state.stack.pop_back();
				}
				state.currentIndent = state.newIndent;
			}
			
			else if (t.type == Token::Name) {
				state.pname = t.s;
				state.pos   = State::Colon;
				
				int N = state.currentIndent - state.newIndent;
				for (int i=0; i < N; ++i)
					state.stack.pop_back();
				state.currentIndent = state.newIndent;
			}
			
			else if (t.type == Token::EndOfLine) {
				
			}
			
			else StateErr(t.type, state.newlines+1, "Name, Whitespace or EndOfLine");
			
			continue;
		}
		
		if (state.pos == State::Colon) {
			if (t.type != Token::Colon)
				StateErr(t.type, newlines+1, "Colon");
			
			state.pos = State::Property;
			continue;
		}
		
		if (state.pos == State::Property) {
			// If newline, start a new table
			if (t.type == Token::EndOfLine) {
				// Push a new table, and increase the indent quantity
				Diatom &d_parent = *state.stack.back();
				Diatom &d_new = d_parent[state.pname] = Diatom();
				state.stack.push_back(&d_new);
				++state.currentIndent;
				
				state.pos = State::StartLine;
			}

			// If a property, add to current table 
			else if (t.type == Token::StringProperty || t.type == Token::NumberProperty || t.type == Token::BoolProperty) {
				Diatom &d = *state.stack.back();
				
				if (t.type == Token::StringProperty)
					d[state.pname] = t.s;
				else if (t.type == Token::NumberProperty)
					d[state.pname] = t.n;
				else if (t.type == Token::BoolProperty)
					d[state.pname] = (t.s == "true");
				
				state.pos = State::PostProperty;
			}
			
			continue;
		}
		
		if (state.pos == State::PostProperty) {
			if (t.type == Token::Comma)
				;
			else if (t.type == Token::EndOfLine)
				state.pos = State::StartLine;
			else
				StateErr(t.type, state.newlines+1, "EndOfLine or EndOfString");
		}
	}
	
	return state.d;
}


Diatom diatomFromFile(const Str &filename) {
	FILE *fp = fopen(filename.c_str(), "r");
	if (!fp) return Diatom::NilObject();
	
	Str s;
	
	char chunk[129];
	int n_bytes_read;
	while ((n_bytes_read = (int)fread(chunk, 1, 128, fp)) > 0) {
		chunk[n_bytes_read] = '\0';
		s += chunk;
	}
	
	fclose(fp);
	return diatomFromString(s);
}

