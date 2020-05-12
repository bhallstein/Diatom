/*
 * LuaObj.cpp - RLTG implementation
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

#include "LuaObj.h"

/* N.S.C. implementation */

bool NumericoidStringComparator::operator()(const std::string &a, const std::string &b) const {
	float x, y;
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

LuaObj LuaObj::_nilobject;


/* LuaObj constr: get an LuaObj representing the object X at -1 on the stack */

LuaObj::LuaObj(lua_State *L) : type(ValueType::NIL)
{
	using std::string;
	
	if (!L) return;
	
	// If we are a simple type, just set type & value
	if (lhIsSimpleType(L, -1)) {
		if (lua_isboolean(L, -1)) {
			type = ValueType::BOOL;
			bool_value = lua_toboolean(L, -1);
		}
		else if (lua_isnumber(L, -1)) {
			type = ValueType::NUMBER;
			number_value = lua_tonumber(L, -1);
		}
		else if (lua_isstring(L, -1)) {
			type = ValueType::STRING;
			str_value = lua_tostring(L, -1);
		}
	}
	
	// If we are a table, recursively get descendants
	else if (lua_istable(L, -1)) {
		type = ValueType::TABLE;
		lua_pushnil(L);					// S: nil, X
		while (lua_next(L, -2)) {		// S: value, key, X
			LuaObj desc(L);
			if (desc.type != ValueType::NIL && lhIsSimpleType(L, -2)) {
				std::string descName = lhSimpleTypeToString(L, -2);
				descendants.insert(_descendantmap::value_type(descName, desc));
			}
			lua_pop(L, 1);				// S: key, X
		}								// S: X
	}
}


/* LuaObj implementation */

LuaObj& LuaObj::operator[] (const char *s) {
	_descendantmap::iterator it = descendants.find(s);
	return (it == descendants.end() ? _nilobject : it->second);
}
LuaObj& LuaObj::operator[] (const std::string &s) {
	return (*this)[s.c_str()];
}

std::string LuaObj::_print() const {
	std::stringstream ss;
	ss << std::string("type: ");
	ss << std::string(
		type == ValueType::TABLE ? "table" :
		type == ValueType::NUMBER ? "number" :
		type == ValueType::BOOL ? "bool" :
		type == ValueType::STRING ? "string" :
		type == ValueType::NIL ? "nil" : "unknown"
	);
	if (type != ValueType::NIL) {
		ss << " value: ";
		if (type == ValueType::NUMBER) ss << number_value;
		else if (type == ValueType::STRING) ss <<  str_value;
		else ss << (bool_value ? "true" : "false");
	}
	else if (type == ValueType::TABLE)
		ss << " descendants: " << descendants.size();
	return ss.str();
}


/* LuaObj static helpers */

bool LuaObj::lhIsSimpleType(lua_State *L, int ind) {
	return lua_isnumber(L, ind) || lua_isboolean(L, ind) || lua_isstring(L, ind);
}
std::string LuaObj::lhSimpleTypeToString(lua_State *L, int ind) {
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
