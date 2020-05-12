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
};

class lua_State;


class LuaObj {
public:
	LuaObj(const std::string &filename, const std::string &objectName);
	LuaObj(lua_State *);	  // Load from the table at -1 on the Lua stack
	LuaObj() : type(Type::_Nil) { }		// Create an empty/nil LuaObj
	
	static bool loadLuaFile(const std::string &filename, lua_State **);
	static bool loadLuaString(const std::string &string, lua_State **);
	static std::string reindentString(const std::string &s);
	
	struct Type {
		enum T { Numeric, Bool, String, Table, _Nil };
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
	bool isNil()    { return type == Type::_Nil; }
	
private:
	void load(lua_State *);				// Populate recursively from lua stack
	
#ifdef LUAOBJ_PRINT
	std::string _print() const;
#endif
	
	static LuaObj _nilobject;
	
};

#endif
