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

#ifndef __LuaObj_h
#define __LuaObj_h

#include <map>
#include <string>


struct NumericoidStringComparator {
	bool operator() (const std::string &a, const std::string &b) const;
	template <typename T>
	static bool _strToT(T &t, const std::string &s);
};

class lua_State;


class LuaObj {
public:
	LuaObj(std::string filename, std::string objectName);
	
	struct Type {
		enum T { Numeric, Bool, String, Table, Nil };
	};
	Type::T type;
	
	float       number_value;
	bool        bool_value;
	std::string str_value;
	
	typedef std::map<std::string, LuaObj, NumericoidStringComparator> _descendantmap;
	_descendantmap descendants;
		// Note: the descendants map is sorted by the N.S.C. so ordering may
		// differ from that in the original lua.
	
	LuaObj& operator[] (const char *);
	LuaObj& operator[] (const std::string &);
	bool isTable()  { return type == Type::Table; }
	bool isNumber() { return type == Type::Numeric; }
	bool isString() { return type == Type::String; }
	bool isBool()   { return type == Type::Bool; }
	bool isNil()    { return type == Type::Nil; }
	
private:
	LuaObj() : type(Type::Nil) { }		// Create an empty/nil LuaObj
	
	void load(lua_State *L);			// Populate recursively from lua stack
	std::string _print() const;
	
	static LuaObj _nilobject;
	
};


#endif
