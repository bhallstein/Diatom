/*
 * Diatom.cpp - RLTG implementation
 *
 * Copyright (c) 2012 - Ben Hallstein - ben.am
 * Published under the MIT license - http://opensource.org/licenses/MIT
 *
 */

#include "Diatom.h"


Diatom Diatom::_nilobject = Diatom(MrNil());


Diatom& Diatom::operator[] (const char *s) {
	auto it = _descendants.find(s);
	if (it == _descendants.end()) return _descendants[s] = _nilobject;
	return it->second;
}
Diatom& Diatom::operator[] (const std::string &s) {
	return (*this)[s.c_str()];
}

void Diatom::clone(const Diatom &from, Diatom *to, bool to_needs_cleanup) {
	using std::string;
	
	if (to_needs_cleanup) {
		switch (to->_type) {
			case Type::String: to->_str_value.~string(); break;
			case Type::Table:  to->_descendants.~map(); break;
			default: break;
		}
	}

	Type::T *_type_unsafe = (Type::T*) &to->_type;
	*_type_unsafe = from._type;

	switch (to->_type) {
		case Type::String: new (&to->_str_value) std::string(from._str_value); break;
		case Type::Table:  new (&to->_descendants) _descendantmap(from._descendants); break;
		case Type::Number:  to->_number_value = from._number_value; break;
		case Type::Bool:    to->_bool_value = from._bool_value; break;
		default: break;
	}
}
