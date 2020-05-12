/*
 * Diatom-Lua.h
 * 
 * - Serialization to lua string
 * - Deserialization from lua file
 *
 *
 * Copyright (c) 2015 - Ben Hallstein - ben.am
 * Published under the MIT license - http://opensource.org/licenses/MIT
 *
 */

#ifndef __DiatomLua_h
#define __DiatomLua_h

#include "Diatom.h"

class lua_State;

std::string diatomToLua(Diatom &d);

Diatom luaToDiatom(const std::string &filename, const std::string &objectName);
Diatom luaToDiatom(lua_State *);

#endif
