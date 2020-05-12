/*
 * Diatom-Lua.cpp
 *
 */

#include "Diatom-Lua.h"
#include "lua.hpp"

std::string __d_to_s(Diatom &d) {
	if (d.isNumber()) return std::to_string(d.number_value());
	if (d.isString()) return std::string("\"") + d.str_value() + "\"";
	if (d.isBool())   return (d.bool_value() ? "true" : "false");
	return "";
}

std::string __reindent_lua_str(const std::string &s) {
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

std::string __diatomToLua(Diatom &d) {
	std::string s;
	s += "{\n";
	for (auto &i : d.descendants()) {
		if (i.second.isNil()) continue;
		s += i.first + std::string(" = ");
		if (i.second.isTable()) s += __diatomToLua(i.second);
		else                    s += __d_to_s(i.second);
		if (i.first != d.descendants().rbegin()->first)
			s += ",\n";
		else
			s += "\n";
	}
	s += "}";
	return s;
}

bool __dlua_IsSimpleType(lua_State *L, int ind) {
	int t = lua_type(L, ind);
	return (t == LUA_TNUMBER || t == LUA_TBOOLEAN || t == LUA_TSTRING);
}
std::string __dlua_SimpleTypeToString(lua_State *L, int ind) {
	std::string s;
	int t = lua_type(L, ind);
	if (t == LUA_TBOOLEAN)     s = lua_toboolean(L, ind) ? "true" : "false";
	else if (t == LUA_TNUMBER) s = std::to_string(lua_tonumber(L, ind));
	else if (t == LUA_TSTRING) s = lua_tostring(L, ind);
	return s;
}

void __dlua_loadFile_Int(const char *fname, lua_State **L) {
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
void __dlua_loadString_Int(const char *str, lua_State **L) {
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
bool __dlua_execute(lua_State **L, const char *identifier) {
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
bool __dlua_loadLuaFile(const std::string &filename, lua_State **L) {
	__dlua_loadFile_Int(filename.c_str(), L);
	return (*L == NULL ? false : __dlua_execute(L, filename.c_str()));
}
bool __dlua_loadLuaString(const std::string &string, lua_State **L) {
	__dlua_loadString_Int(string.c_str(), L);
	return (*L == NULL ? false : __dlua_execute(L, "[loaded from string]"));
}

Diatom __luaToDiatom(lua_State *L) {
	using std::string;
	if (!L) return Diatom::NilObject();
	
	// If we are a simple type, just set type & value
	if (__dlua_IsSimpleType(L, -1)) {
		int t = lua_type(L, -1);
		if (t == LUA_TSTRING)   return lua_tostring(L, -1);
		if (t == LUA_TBOOLEAN)  return (bool) lua_toboolean(L, -1);
		if (t == LUA_TNUMBER)   return (double) lua_tonumber(L, -1);
	}
	
	// If we are a table, recursively get descendants
	else if (lua_istable(L, -1)) {		// S: X
		Diatom d;
		lua_pushnil(L);					// S: nil, X
		while (lua_next(L, -2)) {		// S: value, key, X
			Diatom desc = __luaToDiatom(L);
			if (!desc.isNil() && __dlua_IsSimpleType(L, -2)) {
				std::string descName = __dlua_SimpleTypeToString(L, -2);
				d[descName] = desc;
			}
			lua_pop(L, 1);				// S: key, X
		}								// S: X
		return d;
	}
	
	return Diatom::NilObject();
}


#pragma mark - diatomToLua

std::string diatomToLua(Diatom &d) {
	return d.isTable() ? __reindent_lua_str(__diatomToLua(d)) : __d_to_s(d);
}


#pragma mark - luaToDiatom

Diatom luaToDiatom(const std::string &filename, const std::string &objectname) {
	lua_State *L;
	Diatom d = Diatom::NilObject();
	if (__dlua_loadLuaFile(filename, &L)) {
		lua_getglobal(L, objectname.c_str());
		d = __luaToDiatom(L);
		lua_close(L);
	}
	return d;
}
Diatom luaToDiatom(lua_State *L) {
	return __luaToDiatom(L);
}
