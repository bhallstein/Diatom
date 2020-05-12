/*
 * Diatom.h - RLTG implementation
 *
 * Copyright (c) 2012 - Ben Hallstein - ben.am
 * - Converted from lua-specific to generic table object on 18.4.2015
 *   on the train from Kolding to Copenhagen.
 * 
 * Published under the MIT license - http://opensource.org/licenses/MIT
 *
 */

#ifndef __Diatom_h
#define __Diatom_h

#include <vector>
#include <string>
#include "StretchyArray_NonReallocating.h"


class Diatom {
public:
	struct Type {
		enum T { Number, Bool, String, Table, _Nil };
	};
	
	template <class T>
	struct Wrapper {
		std::string s;
		T d;
	};
	typedef StretchyArray_NonReallocating<Wrapper<Diatom>> _descvec;
	
	Diatom(double x) : _type(Type::Number), _number_value(x) {  }   // Numeric
	Diatom(bool x)   : _type(Type::Bool),   _bool_value(x) {  }     // Bool
	Diatom(const char *s) : _type(Type::String)                     // String (char*)
	{  new (&_str_value) std::string(s); }

	Diatom(const std::string &s) : _type(Type::String)              // String (std::string)
	{  new (&_str_value) std::string(s);  }

	Diatom() : _type(Type::Table)
	{  new (&_descendants) _descvec();  }
	
	Diatom(const Diatom &l) : _type(l._type)
	{
		clone(l, this, false);
	}
	Diatom& operator=(const Diatom &l)
	{
		clone(l, this);
		return *this;
	}
	static Diatom NilObject() { return _nilobject; }
	
	~Diatom()
	{
		using std::string;
		if (isString()) _str_value.~string();
		else if (isTable()) _descendants.~StretchyArray_NonReallocating();
	}
	
	Diatom& operator[] (const char *);
	Diatom& operator[] (const std::string &);
	bool isTable()  const { return _type == Type::Table;  }
	bool isNumber() const { return _type == Type::Number; }
	bool isString() const { return _type == Type::String; }
	bool isBool()   const { return _type == Type::Bool;   }
	bool isNil()    const { return _type == Type::_Nil;   }
	
	Type::T type() { return _type; }
	size_t n_descendants() const { return _descendants.current_size(); }
	
	const double &        number_value() const { return _number_value; }
	const bool &          bool_value()   const { return _bool_value; }
	const std::string &   str_value()    const { return _str_value; }
	
	template<class Fn>
	void each_descendant(Fn f) {
		_descendants.each([&](Wrapper<Diatom> &w) {
			f(w.s, w.d);
		});
	}
	
	// void print() {
	// 	if (isTable())
	// 		each_descendant([&](std::string key, Diatom &d) {
	// 			printf("%s: ", key.c_str());
	// 			d.print();
	// 		});
	// 	else if (isNumber())
	// 		printf("%.1f\n", number_value());
	// 	else if (isString())
	// 		printf("%s\n", str_value().c_str());
	// 	else if (isBool())
	// 		printf("%s\n", bool_value() ? "true" : "false");
	// 	else
	// 		printf("nil\n");
	// }
	
private:
	const Type::T _type;
	static Diatom _nilobject;
	struct MrNil { };
	Diatom(MrNil) : _type(Type::_Nil) {  }
	
	union {
		double       _number_value;
		bool         _bool_value;
		std::string  _str_value;
		_descvec     _descendants;
	};
	
	static void clone(const Diatom &from, Diatom *to, bool to_needs_cleanup = true);
		// Clone 'from' onto 'to'.
		//  - Deals with the conditional memory de/allocation requirements of the union.
		//  - If 'to' is not initialized yet, set final param to false (i.e. in copy constructor).
};

#endif
