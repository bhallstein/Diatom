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


class LuaObj {
public:
	struct Type {
		enum T { Number, Bool, String, Table, _Nil };
	};
	typedef std::map<std::string, LuaObj, NumericoidStringComparator> _descendantmap;
	
	LuaObj(double x) : _type(Type::Number), _number_value(x) {  }   // Numeric
	LuaObj(bool x)   : _type(Type::Bool),   _bool_value(x) {  }     // Bool
	LuaObj(const char *s) : _type(Type::String)                     // String (char*)
	{  new (&_str_value) std::string(s); }

	LuaObj(const std::string &s) : _type(Type::String)              // String (std::string)
	{  new (&_str_value) std::string(s);  }

	LuaObj() : _type(Type::Table)
	{  new (&_descendants) _descendantmap();  }
	
	LuaObj(const LuaObj &l) : _type(l._type)
	{
		clone(l, this, false);
	}
	LuaObj& operator=(const LuaObj &l)
	{
		clone(l, this);
		return *this;
	}
	static LuaObj NilObject() { return _nilobject; }
	
	~LuaObj()
	{
		using std::string;
		if (isString()) _str_value.~string();
		else if (isTable()) _descendants.~map();
	}
	
	LuaObj& operator[] (const char *);
	LuaObj& operator[] (const std::string &);
	bool isTable()  const { return _type == Type::Table; }
	bool isNumber() const { return _type == Type::Number; }
	bool isString() const { return _type == Type::String; }
	bool isBool()   const { return _type == Type::Bool; }
	bool isNil()    const { return _type == Type::_Nil; }
	
	const Type::T & type() { return _type; }
	
	const double &         number_value() { return _number_value; }
	const bool &           bool_value()   { return _bool_value; }
	const std::string &    str_value()    { return _str_value; }
	_descendantmap &       descendants()  { return _descendants; }
	
private:
	const Type::T _type;
	static LuaObj _nilobject;
	struct MrNil { };
	LuaObj(MrNil) : _type(Type::_Nil) {  }
	
	union {
		double         _number_value;
		bool           _bool_value;
		std::string    _str_value;
		_descendantmap _descendants;
			// Note: the descendants map is sorted by the N.S.C., so ordering may
			// differ from that in the original lua.
	};
	
	static void clone(const LuaObj &from, LuaObj *to, bool to_needs_cleanup = true);
		// Clone 'from' onto 'to'.
		//  - Deals with the conditional memory de/allocation requirements of the union.
		//  - If 'to' is not initialized yet, set final param to false (i.e. in copy constructor).
	
public:
#ifdef LUAOBJ_PRINT
	void _print(int indentLevel = 0) const {
		printf("%s:",
			isTable()   ? "table" :
			isNumber()  ? "number" :
			isBool()    ? "bool" :
			isString()  ? "string" :
			isNil()     ? "nil" : "unknown");
	
		if (isTable()) {
			printf(" n_descendants: %lu\n", _descendants.size());
			indentLevel++;
			for (auto &i : _descendants) {
				for (int i=0; i < indentLevel; ++i)
					printf("    ");
				printf("%s - ", i.first.c_str());
				i.second._print(indentLevel);
			}
		}
		else if (!isNil()) {
			printf(" value: ");
			if (isNumber()) printf("%f", _number_value);
			else if (isString()) printf("%s", _str_value.c_str());
			else printf("%s", _bool_value ? "true" : "false");
			printf("\n");
		}
	}
#endif
};

#endif
