/*
 * LuaObj.h - RLTG implementation
 *
 * Copyright (C) 2012 Ben Hallstein
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __W__LuaObj
#define __W__LuaObj

#include "lua.hpp"

#include <iostream>
#include <map>
#include <sstream>


struct NumericoidStringComparator {
	bool operator() (const std::string &a, const std::string &b);
	template <typename T>
	bool _strToT(T &t, const std::string &s);
};

class LuaObj {
public:
	LuaObj(lua_State *);
		// Create a recursive LuaObj representing whatever is at -1
		// on the supplied lua stack.
	
	enum ValueType {
		NUMBER, BOOL, STRING, TABLE, NIL
	};
	
	ValueType type;
	float       number_value;
	bool        bool_value;
	std::string str_value;
	
	typedef std::map<std::string, LuaObj, NumericoidStringComparator> _descendantmap;
	_descendantmap descendants;
		// Note: the descendants map is sorted by the N.S.C. so ordering may
		// differ from that in the original lua.
	
	LuaObj& operator[] (const char *);
	bool isTable()  { return type == ValueType::TABLE; }
	bool isNumber() { return type == ValueType::NUMBER; }
	bool isString() { return type == ValueType::STRING; }
	bool isBool()   { return type == ValueType::BOOL; }
	bool isNil()    { return type == ValueType::NIL; }
private:
	LuaObj() : type(ValueType::NIL) { }
		// Create an empty/nil LuaObj

	std::string _print() const;
	static LuaObj _nilobject;
	
	static bool lhIsSimpleType(lua_State *, int ind);
	static std::string lhSimpleTypeToString(lua_State *, int ind);
};

#endif
