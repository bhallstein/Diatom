/*
 * Diatom.cpp - RLTG implementation
 *
 * Copyright (c) 2012 - Ben Hallstein - ben.am
 * Published under the MIT license - http://opensource.org/licenses/MIT
 *
 */

#include "Diatom.h"
#include <vector>
using Str = std::string;


#pragma mark - Diatom

Diatom Diatom::_nilobject = Diatom(MrNil());


Diatom& Diatom::operator[] (const char *s) {
	int i = _descendants.find([=](Wrapper<Diatom> &w) {
		return w.s == s;
	});
	if (i == -1) {
		_descendants.push({ s, _nilobject });
		return _descendants.last().d;
	}
	return _descendants[i].d;
}
Diatom& Diatom::operator[] (const Str &s) {
	return (*this)[s.c_str()];
}

void Diatom::clone(const Diatom &from, Diatom *to, bool to_needs_cleanup) {
	if (to_needs_cleanup) {
		switch (to->_type) {
			case Type::String: to->_str_value.~Str(); break;
			case Type::Table:  to->_descendants.~StretchyArray_NonReallocating(); break;
			default: break;
		}
	}

	Type::T *_type_unsafe = (Type::T*) &to->_type;
	*_type_unsafe = from._type;

	switch (to->_type) {
		case Type::String:  {  new (&to->_str_value) Str(from._str_value); break;           }
		case Type::Table:   {  new (&to->_descendants) _descvec(from._descendants); break;  }
		case Type::Number:  {  to->_number_value = from._number_value; break;               }
		case Type::Bool:    {  to->_bool_value = from._bool_value; break;                   }
		default: break;
	}
}
