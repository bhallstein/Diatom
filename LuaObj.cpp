/*
 * LuaObj.cpp - RLTG implementation
 *
 * Copyright (c) 2012 - Ben Hallstein - ben.am
 * Published under the MIT license - http://opensource.org/licenses/MIT
 *
 */

#include "LuaObj.h"
#include "lua.hpp"
#include <sstream>

#pragma mark Helpers

bool __luaobj_IsSimpleType(lua_State *L, int ind) {
	int t = lua_type(L, ind);
	return (t == LUA_TNUMBER || t == LUA_TBOOLEAN || t == LUA_TSTRING);
}
std::string __luaobj_SimpleTypeToString(lua_State *L, int ind) {
	std::string s;
	int t = lua_type(L, ind);
	if (t == LUA_TBOOLEAN)
		s = lua_toboolean(L, ind) ? "true" : "false";
	else if (t == LUA_TNUMBER) {
		std::stringstream ss;
		ss << lua_tonumber(L, ind);
		s = ss.str();
	}
	else if (t == LUA_TSTRING)
		s = lua_tostring(L, ind);
	return s;
}
template <typename T>
bool __luaobj_strToT(T &t, const std::string &s) {
	return !(std::istringstream(s) >> t).fail();
}

void __luaobj_loadFile(const char *fname, lua_State **L) {
	*L = luaL_newstate();
	bool loadError = luaL_loadfile(*L, fname);
	if (loadError) {
		printf(
			   "Couldn't open lua file '%s' - error was: %s\n",
			   fname,
			   lua_tostring(*L, -1)
			   );
		lua_close(*L);
		*L = NULL;
	}
}
void __luaobj_loadString(const char *str, lua_State **L) {
	*L = luaL_newstate();
	bool loadError = luaL_loadstring(*L, str);
	if (loadError) {
		printf(
			   "Couldn't open lua string '%s' - error was: %s\n",
			   str,
			   lua_tostring(*L, -1)
			   );
		lua_close(*L);
		*L = NULL;
	}
}
bool __luaobj_execute(lua_State **L, const char *identifier) {
	bool runError = lua_pcall(*L, 0, 0, 0);
	if (runError) {
		printf(
			   "Couldn't execute lua '%s' - error was: %s\n",
			   identifier,
			   lua_tostring(*L, -1)
			   );
		lua_close(*L);
	}
	return !runError;
}


#pragma mark - NumericoidStringComparator

bool NumericoidStringComparator::operator()(const std::string &a, const std::string &b) const {
	double x, y;
	bool a_numeric = __luaobj_strToT(x, a);
	bool b_numeric = __luaobj_strToT(y, b);
	if (a_numeric && !b_numeric) return true;
	else if (!a_numeric && b_numeric) return false;
	else if (a_numeric && b_numeric) return (x < y);
	return (a < b);
}


#pragma mark - LuaObj

const LuaObj LuaObj::_nilobject;

LuaObj::LuaObj(const std::string &filename, const std::string &objectname) :
	_type(Type::_Nil)
{
	lua_State *L;
	if (loadLuaFile(filename, &L)) {
		lua_getglobal(L, objectname.c_str());
		*this = load(L);
		lua_close(L);
	}
}
LuaObj::LuaObj(lua_State *L) :
	_type(Type::_Nil)
{
	*this = load(L);
}

bool LuaObj::loadLuaFile(const std::string &filename, lua_State **L) {
	__luaobj_loadFile(filename.c_str(), L);
	return (*L == NULL ? false : __luaobj_execute(L, filename.c_str()));
}
bool LuaObj::loadLuaString(const std::string &string, lua_State **L) {
	__luaobj_loadString(string.c_str(), L);
	return (*L == NULL ? false : __luaobj_execute(L, "[loaded from string]"));
}


LuaObj LuaObj::load(lua_State *L)
{
	using std::string;
	
	if (!L) return _nilobject;
	
	// If we are a simple type, just set type & value
	if (__luaobj_IsSimpleType(L, -1)) {
		int t = lua_type(L, -1);
		if (t == LUA_TSTRING)   return LuaObj(lua_tostring(L, -1));
		if (t == LUA_TBOOLEAN)  return LuaObj((bool) lua_toboolean(L, -1));
		if (t == LUA_TNUMBER)   return LuaObj((double) lua_tonumber(L, -1));
	}
	
	// If we are a table, recursively get descendants
	else if (lua_istable(L, -1)) {		// S: X
		LuaObj l;
		lua_pushnil(L);					// S: nil, X
		while (lua_next(L, -2)) {		// S: value, key, X
			LuaObj desc = load(L);
			if (!desc.isNil() && __luaobj_IsSimpleType(L, -2)) {
				std::string descName = __luaobj_SimpleTypeToString(L, -2);
				l._descendants.insert(_descendantmap::value_type(descName, desc));
			}
			lua_pop(L, 1);				// S: key, X
		}								// S: X
		return l;
	}
	
	return _nilobject;
}


LuaObj& LuaObj::operator[] (const char *s) {
	// auto it = descendants.find(s);
	// return (it == descendants.end() ? _nilobject : it->second);
	return _descendants[s];
}
LuaObj& LuaObj::operator[] (const std::string &s) {
	return (*this)[s.c_str()];
}

#ifdef LUAOBJ_PRINT
void LuaObj::_print() const {
	std::stringstream ss;
	ss << std::string("type: ");
	ss << std::string(
		isTable()   ? "table" :
		isNumber()  ? "number" :
		isBool()    ? "bool" :
		isString()  ? "string" :
		isNil()     ? "nil" : "unknown"
	);
	
	// if (type == Type::String) ss << str_value;
	
	if (isTable())
		ss << " n_descendants: " << _descendants.size();
	else if (!isNil()) {
		ss << " value: ";
		if (isNumber()) ss << _number_value;
		else if (isString()) ss << _str_value;
		else ss << (_bool_value ? "true" : "false");
	}

	printf("%s\n", ss.str().c_str());
}
#endif


// Reindenting function for lua

std::string LuaObj::reindentString(const std::string &s) {
	std::string t;
	int indentLevel = 0;
	bool insideString = false;
	
	for (auto it = s.begin(), str_begin = s.begin(), str_end = s.end(); it != str_end; ++it) {
		bool last  = (it+1 == str_end);
		bool first = (it == str_begin);
		char c = *it, cp = ' ', cn = ' ';
		
		if (!first) cp = *(it-1);
		if (!last) cn = *(it+1);
		
		if (!insideString && c == '"')              insideString = true;
		if (insideString && c == '"' && cp != '\\') insideString = false;
		
		if (c == '{' && !insideString)           ++indentLevel;
		if (!last && cn == '}' && !insideString) --indentLevel;
		
		t += c;
		if (c == '\n' && !insideString)
			for (int i=0; i < indentLevel; ++i)
				t += "    ";
	}
	return t;
}

