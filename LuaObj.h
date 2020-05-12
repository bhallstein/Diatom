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
	struct Type {
		enum T { Numeric, Bool, String, Table, _Nil };
	};
	typedef std::map<std::string, LuaObj, NumericoidStringComparator> _descendantmap;
	
	LuaObj(const std::string &filename, const std::string &objectName);
	LuaObj(lua_State *);				// Load from the table at -1 on the Lua stack

	LuaObj(double x) : _type(Type::Numeric), _number_value(x) {  }	// Numeric
	LuaObj(bool x)   : _type(Type::Bool),    _bool_value(x) {  }	// Bool
	LuaObj(const char *s) : _type(Type::String)						// String (char*)
	{
		new (&_str_value) std::string(s);
	}
	LuaObj(const std::string &s) : _type(Type::String)				// String (std::string)
	{
		new (&_str_value) std::string(s);
	}
	LuaObj() : _type(Type::Table)
	{
		new (&_descendants) _descendantmap();
	}
	
	LuaObj(const LuaObj &l) : _type(l._type)
	{
		clone(l, this);
	}
	LuaObj& operator=(const LuaObj &l)
	{
		clone(l, this);
		return *this;
	}
	LuaObj NilObject() { return _nilobject; }
	
	~LuaObj()
	{
		using std::string;
		if (isString()) _str_value.~string();
		else if (isTable()) _descendants.~map();
	}
	
	static bool loadLuaFile(const std::string &filename, lua_State **);
	static bool loadLuaString(const std::string &string, lua_State **);
	static std::string reindentString(const std::string &s);
	
	LuaObj& operator[] (const char *);
	LuaObj& operator[] (const std::string &);
	bool isTable()  const { return _type == Type::Table; }
	bool isNumber() const { return _type == Type::Numeric; }
	bool isString() const { return _type == Type::String; }
	bool isBool()   const { return _type == Type::Bool; }
	bool isNil()    const { return _type == Type::_Nil; }
	
	const Type::T & type() { return _type; }
	
	const double &         number_value() { return _number_value; }
	const bool &           bool_value()   { return _bool_value; }
	const std::string &    str_value()    { return _str_value; }
	const _descendantmap & descendants()  { return _descendants; }
	
#ifdef LUAOBJ_PRINT
	std::string _print() const;
#endif
	
private:
	static LuaObj load(lua_State *);		// Populate recursively from lua stack
	
	const Type::T _type;
	static const LuaObj _nilobject;
	
	union {
		double         _number_value;
		bool           _bool_value;
		std::string    _str_value;
		_descendantmap _descendants;
			// Note: the descendants map is sorted by the N.S.C., so ordering may
			// differ from that in the original lua.
	};
	
	static void clone(const LuaObj &from, LuaObj *to) {
		Type::T *_type_unsafe = (Type::T*) &to->_type;
		*_type_unsafe = from._type;
		switch (to->_type) {
			case Type::String: new (&to->_str_value) std::string(from._str_value); break;
			case Type::Table:  new (&to->_descendants) _descendantmap(from._descendants); break;
			case Type::Numeric: to->_number_value = from._number_value;
			case Type::Bool:    to->_bool_value = from._bool_value;
			default: break;
		}
	}
	
};

#endif
