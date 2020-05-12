/*
 * Diatom-Storage.h
 * 
 * - Serialization to/from Diatom's format:
 * 
 *    name:
 *       p1: 7,          # comment
 *       p2: "Oh hai",   # commas are optional
 *       p3: true,       # indenting is set by the first indented item
 *       p4:
 *          monkeys: 7
 * 
 *    # names must begin [a-zA-Z], and contain only alphanumeric and _.
 * 
 * Copyright (c) 2015 - Ben Hallstein - ben.am
 * Published under the MIT license - http://opensource.org/licenses/MIT
 *
 */

#ifndef __DiatomStorage_h
#define __DiatomStorage_h

#include "Diatom.h"

std::string diatomToString(Diatom &d, const std::string &name);
std::string diatomToString(Diatom &d);

Diatom diatomFromFile(const std::string &filename);
Diatom diatomFromString(const std::string &);

#endif

