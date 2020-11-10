//
// Diatom.h
//
// General purpose object for storing data:
//  - Number
//  - String
//  - Bool
//  - Empty
//  - Table (other Diatom objects)
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
    enum T { Number, Bool, String, Table, Empty };
  };

  template <class T>
  struct __TableEntry {
    std::string name;
    T item;
  };
  typedef __TableEntry<Diatom> TableEntry;
  typedef std::vector<TableEntry> TableEntryVector;


  // Properties
  // -----------------------------

  Type::T type;
  double           number_value;
  bool             bool_value;
  std::string      string_value;
  TableEntryVector table_entries;

  bool is_empty()  { return type == Type::Empty;  }
  bool is_number() { return type == Type::Number; }
  bool is_bool()   { return type == Type::Bool;   }
  bool is_string() { return type == Type::String; }
  bool is_table()  { return type == Type::Table;  }


  // Constructors
  // -----------------------------

  Diatom()                     : type(Type::Table) { };
  Diatom(double x)             : type(Type::Number), number_value(x) { }
  Diatom(bool x)               : type(Type::Bool),   bool_value(x)   { }
  Diatom(const char *s)        : type(Type::String), string_value(s) { }
  Diatom(const std::string &s) : type(Type::String), string_value(s) { }
  Diatom(Type::T t) : type(t) { }


  // Table diatom lookup
  // -----------------------------

  TableEntryVector::iterator index_of(const std::string &s) {
    return std::find_if(table_entries.begin(), table_entries.end(), [=](const TableEntry &item) {
      return item.name == s;
    });
  }

  Diatom& operator[](const std::string &s) {
    const TableEntryVector::iterator &it = index_of(s);

    if (it == table_entries.end()) {
      table_entries.push_back({ s, Diatom(Type::Empty) });
      return table_entries.back().item;
    }

    return it->item;
  }

  void remove_child(std::string s) {
    auto i = index_of(s);
    if (i != table_entries.end()) {
      table_entries.erase(i);
    }
  }


  // Iteration
  // -----------------------------

  template <class F>
  void each(F f) {
    for (TableEntry &entry : table_entries) {
      f(entry.name, entry.item);
    }
  }

  template <class F>
  void recurse(F f, bool include_top = false) {
    if (include_top) {
      f("", *this);
    }
    for (TableEntry &entry : table_entries) {
      f(entry.name, entry.item);
      if (entry.item.type == Type::Table) {
        entry.item.recurse(f);
      }
    }
  }


  // Other
  // -----------------------------

  std::string type_string() {
    return (
      type == Type::Number ? "Number" :
      type == Type::String ? "String" :
      type == Type::Bool   ? "Bool"   :
      type == Type::Table  ? "Table"  :
      type == Type::Empty  ? "Empty"  : "Unknown"
    );
  };
};


#endif

