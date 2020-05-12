//
// Diatom.h
//
// General purpose object for storing data:
//  - Number
//  - String
//  - Bool
//  - Nil
//  - Nested objects ('Table')
//
// -- MIT Licensed: http://opensource.org/licenses/MIT/
// -- BH 2012
//

#ifndef __Diatom_h
#define __Diatom_h

#include <string>
#include <vector>
#include <algorithm>


struct Diatom {
  struct Type {
    enum T { Number, Bool, String, Table, Nil };
  };

  template <class T>
  struct TableEntry {
    std::string name;
    T item;
  };
  typedef TableEntry<Diatom> DTableEntry;
  typedef std::vector<DTableEntry> TableEntryVector;


  // Properties
  // -----------------------------

  Type::T type;
  double           value__number;
  bool             value__bool;
  std::string      value__string;
  TableEntryVector descendants;

  bool is_nil()    { return type == Type::Nil;   }
  bool is_number() { return type == Type::Number; }
  bool is_bool()   { return type == Type::Bool;   }
  bool is_string() { return type == Type::String; }
  bool is_table()  { return type == Type::Table;  }


  // Constructors
  // -----------------------------

  Diatom()                     : type(Type::Table) { };
  Diatom(double x)             : type(Type::Number), value__number(x) { }
  Diatom(bool x)               : type(Type::Bool),   value__bool(x)   { }
  Diatom(const char *s)        : type(Type::String), value__string(s) { }
  Diatom(const std::string &s) : type(Type::String), value__string(s) { }
  Diatom(Type::T t) : type(t) { }


  // Table diatom lookup
  // -----------------------------

  TableEntryVector::iterator index_of(const std::string &s) {
    return std::find_if(descendants.begin(), descendants.end(), [=](const DTableEntry &item) {
      return item.name == s;
    });
  }

  Diatom& operator[](const std::string &s) {
    const TableEntryVector::iterator &it = index_of(s);

    if (it == descendants.end()) {
      descendants.push_back({ s, Diatom(Type::Nil) });
      return descendants.back().item;
    }

    return it->item;
  }


  // Iteration
  // -----------------------------

  template <class F>
  void each(F f) {
    for (DTableEntry &entry : descendants) {
      f(entry.name, entry.item);
    }
  }

  template <class F>
  void recurse(F f) {
    for (DTableEntry &entry : descendants) {
      f(entry.name, entry.item);

      if (entry.item.type == Type::Table) {
        entry.item.recurse(f);
      }
    }
  }
};


#endif

