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

#pragma mark - Key sorting

#define CH_NUMERIC(c)  (c >= '0' && c <= '9')

struct __SortChunkEntry {
	bool numeric;
	Str s;
};

using Vec = std::vector<__SortChunkEntry>;

Vec __str_to_chunks(const Str &s) {
	Vec chunkz;
	bool exp_numeric = CH_NUMERIC(s[0]);
	Str cur;
	for (auto c : s)
		if (CH_NUMERIC(c) == exp_numeric)
			cur += c;
		else {
			chunkz.push_back({exp_numeric, cur});
			cur = "";
			cur += c;
			exp_numeric = !exp_numeric;
		}
	if (cur.size() > 0)
		chunkz.push_back({exp_numeric, cur});
	return chunkz;
}

bool __chunkLessThan_AtIndex(Vec &v1, Vec &v2, int i) {
	bool i1_gtr = (i >= v1.size());
	bool i2_gtr = (i >= v2.size());
	
	// If i1 is past the end of V1 but V2 has further sections, V1 orders before V2, & vice versa
	if (i1_gtr != i2_gtr) return i1_gtr;
	
	__SortChunkEntry &x1 = v1[i];
	__SortChunkEntry &x2 = v2[i];
	
	// If x1 is numeric and x2 isn't, V1 orders before V2
	if (x1.numeric != x2.numeric) return x1.numeric;
	
	// If neither are numeric, perform a string comparison
	if (!x1.numeric && !x2.numeric) return x1.s < x2.s;
	
	// If both are numeric, perform a numeric comparison
	Str *longer = &x1.s, *shorter = &x2.s;
	if (longer->size() < shorter->size()) longer = &x2.s, shorter = &x1.s;
	
	Str zero_padding = "";
	for (int j=0, n = longer->size() - shorter->size(); j < n; ++j)
		zero_padding += "0";
	*shorter = zero_padding + *shorter;
		
	for (int j=0, n = longer->size(); j < n; ++j) {
		char c = x1.s[j];
		char d = x2.s[j];
		
		if (c == d) continue;
		return (c < d);
	}
	return false;
}

bool Diatom::NumericStringComparator::operator() (const Str &a, const Str &b) const {
	Vec v1 = __str_to_chunks(a);
	Vec v2 = __str_to_chunks(b);
	
	int i, n;
	{
		int l1 = (int)v1.size(), l2 = (int)v2.size();
		n = (l1 > l2 ? l1 : l2);
	}
	
	for (i=0; i < n; ++i)
		if (__chunkLessThan_AtIndex(v1, v2, i))
			return true;
	
	return false;
}


#pragma mark - Diatom

Diatom Diatom::_nilobject = Diatom(MrNil());


Diatom& Diatom::operator[] (const char *s) {
	auto it = _descendants.find(s);
	if (it == _descendants.end()) return _descendants[s] = _nilobject;
	return it->second;
}
Diatom& Diatom::operator[] (const Str &s) {
	return (*this)[s.c_str()];
}

void Diatom::clone(const Diatom &from, Diatom *to, bool to_needs_cleanup) {
	if (to_needs_cleanup) {
		switch (to->_type) {
			case Type::String: to->_str_value.~Str(); break;
			case Type::Table:  to->_descendants.~map(); break;
			default: break;
		}
	}

	Type::T *_type_unsafe = (Type::T*) &to->_type;
	*_type_unsafe = from._type;

	switch (to->_type) {
		case Type::String:  new (&to->_str_value) Str(from._str_value); break;
		case Type::Table:   new (&to->_descendants) _descendantmap(from._descendants); break;
		case Type::Number:  to->_number_value = from._number_value; break;
		case Type::Bool:    to->_bool_value = from._bool_value; break;
		default: break;
	}
}
