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
	return (lua_isnumber(L, ind) || lua_isboolean(L, ind) || lua_isstring(L, ind));
} 
std::string __luaobj_SimpleTypeToString(lua_State *L, int ind) {
	std::string s;
	if (lua_isboolean(L, ind))
		s = lua_toboolean(L, ind) ? "true" : "false";
	else if (lua_isnumber(L, ind)) {
		std::stringstream ss;
		ss << lua_tonumber(L, ind);
		s = ss.str();
	}
	else if (lua_isstring(L, ind))
		s = lua_tostring(L, ind);
	return s;
}
bool __luaobj_luaLoad(const std::string &filename, lua_State **L) {
	*L = luaL_newstate();
	bool loadError = luaL_loadfile(*L, filename.c_str());
	if (loadError) {
		printf(
			"Couldn't open lua file '%s' - error was: %s\n",
				filename.c_str(),
				lua_tostring(*L, -1)
		);
		lua_close(*L);
		return false;
	}
	bool runError = lua_pcall(*L, 0, 0, 0);
	if (runError) {
		printf(
			"Couldn't execute file '%s' - error was: %s\n",
				filename.c_str(),
				lua_tostring(*L, -1)
		);
		lua_close(*L);
		return false;
	}
	return true;
}


#pragma mark - NumericoidStringComparator

bool NumericoidStringComparator::operator()(const std::string &a, const std::string &b) const {
	double x, y;
	bool a_numeric = _strToT(x, a), b_numeric = _strToT(y, b);
	if (a_numeric && !b_numeric) return true;
	else if (!a_numeric && b_numeric) return false;
	else if (a_numeric && b_numeric) return (x < y);
	return (a < b);
}
template <typename T>
bool NumericoidStringComparator::_strToT(T &t, const std::string &s) {
	return !(std::istringstream(s) >> t).fail();
}


#pragma mark - LuaObj

LuaObj LuaObj::_nilobject;

LuaObj::LuaObj(std::string filename, std::string objectname) :
	type(Type::Nil)
{
	lua_State *L;
	if (__luaobj_luaLoad(filename, &L)) {
		lua_getglobal(L, objectname.c_str());
		load(L);
		lua_close(L);
	}
}
LuaObj::LuaObj(lua_State *L) :
	type(Type::Nil)
{
	load(L);
}

void LuaObj::load(lua_State *L)
{
	using std::string;
	
	if (!L) return;
	
	// If we are a simple type, just set type & value
	if (__luaobj_IsSimpleType(L, -1)) {
		if (lua_isboolean(L, -1)) {
			type = Type::Bool;
			bool_value = lua_toboolean(L, -1);
		}
		else if (lua_isnumber(L, -1)) {
			type = Type::Numeric;
			number_value = lua_tonumber(L, -1);
		}
		else if (lua_isstring(L, -1)) {
			type = Type::String;
			str_value = lua_tostring(L, -1);
		}
	}
	
	// If we are a table, recursively get descendants
	else if (lua_istable(L, -1)) {
		type = Type::Table;
		lua_pushnil(L);					// S: nil, X
		while (lua_next(L, -2)) {		// S: value, key, X
			LuaObj desc;
			desc.load(L);
			if (desc.type != Type::Nil && __luaobj_IsSimpleType(L, -2)) {
				std::string descName = __luaobj_SimpleTypeToString(L, -2);
				descendants.insert(_descendantmap::value_type(descName, desc));
			}
			lua_pop(L, 1);				// S: key, X
		}								// S: X
	}
}


LuaObj& LuaObj::operator[] (const char *s) {
	auto it = descendants.find(s);
	return (it == descendants.end() ? _nilobject : it->second);
}
LuaObj& LuaObj::operator[] (const std::string &s) {
	return (*this)[s.c_str()];
}

std::string LuaObj::_print() const {
	std::stringstream ss;
	ss << std::string("type: ");
	ss << std::string(
		type == Type::Table ? "table" :
		type == Type::Numeric ? "number" :
		type == Type::Bool ? "bool" :
		type == Type::String ? "string" :
		type == Type::Nil ? "nil" : "unknown"
	);
	
	if (type == Type::Table)
		ss << " n_descendants: " << descendants.size();
	else if (type != Type::Nil) {
		ss << " value: ";
		if (type == Type::Numeric) ss << number_value;
		else if (type == Type::String) ss <<  str_value;
		else ss << (bool_value ? "true" : "false");
	}

	return ss.str();
}

