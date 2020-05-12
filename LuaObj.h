/*
 * LuaObj.h - RLTG implementation
 *
 * Copyright (c) 2012 - Ben Hallstein - ben.am
 * Published under the MIT license - http://opensource.org/licenses/MIT
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
	LuaObj(lua_State *);	// Load from the table at -1 on the Lua stack
	
	struct Type {
		enum T { Numeric, Bool, String, Table, Nil };
	};
	Type::T type;
	
	double      number_value;
	bool        bool_value;
	std::string str_value;
	
	typedef std::map<std::string, LuaObj, NumericoidStringComparator> _descendantmap;
	_descendantmap descendants;
		// Note: the descendants map is sorted by the N.S.C., so ordering may
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
	
	void load(lua_State *);				// Populate recursively from lua stack
	std::string _print() const;
	
	static LuaObj _nilobject;
	
};


#endif
