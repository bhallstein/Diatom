
# About Diatom

Diatom is a simple table class, which can either hold a value, or a table of “descendants” (other Diatoms).


## Types

Diatom support only three types of value – numbers (i.e. doubles), strings, and bools. The can also be nil – i.e. not holding a value. You can check the type, & obtain the value, like so:

```cpp
Diatom d(2.718);
d.isNumber();    // => true
double exp = d.number_value();    // exp == 2.718
```

Instead of representing a numeric, bool or string value, a *table* Diatom has a set of named descendants, which are other Diatoms, which are indexed by string:

```cpp
Diatom d;
d.isTable();    // => true
d["exp"] = Diatom(exp);    // d["exp"] is a new number-Diatom
                           // (you could also simply write d["exp"] = exp)
table_diatom["exp"].number_value();    // => 2.718
```


## Copyability & Immutability

All Diatoms can be copied, including tables, which deep-copy.

Diatoms are semi-immutable – their values cannot be set directly after construction. However, they can be altered by copying from another Diatom:

```cpp
Diatom d1(3.14);
d1 = Diatom("Architeuthis");
d1.str_value();    // => "Architeuthis"
```

Simple assignment is possible, due to implicit conversion – consider this syntactic sugar:

```cpp
d1 = "Archaeteuthis";
```
    
Here, C++ constructs a temporary Diatom from the C string, then calls d1’s `operator=`, copying the temporary into d1, which is now a string Diatom.


## However, tables are mutable

The exception, to allow adding and removing descendants, is table Diatoms. The following is perfectly valid:

```cpp
Diatom seabird_genuses;
table["shearwater"] = "Puffinus";
table["cormorant"]  = "Phalacrocorax";
table["puffin"]     = "Fratercula";
```


## Constructing 

```cpp
Diatom()
```
    
The default constructor creates an empty table Diatom.

```cpp 
Diatom(double) / Diatom(bool) / Diatom(string or const char *)
```

Create number, bool, or string Diatoms.

There is no constructor to create a nil Diatom. Use the static method:

```cpp
Diatom d_nil = Diatom::NilObject();
```

## Methods

```cpp
bool isTable()
isNumber()
isBool()
isString()
isNil()
```

These are self-explanatory, hopefully.

```cpp
Diatom::Type::T type()
```

Return the Diatom’s type as an enum. The possible values of the enum are: `Number` / `Bool` / `String` / `Table` / `_Nil`.

```cpp
Diatom& operator[](std::string s or const char *s)
```

If a table, get the descendant named `s`.

```cpp
const double &           number_value()
const bool &             bool_value()
const std::string &      str_value()
Diatom::_descendantmap & descendants()
```

Get the value, or the map of descendants.


## Kthx

That‘s it. Diatom is pretty small.

Diatom in its current form was written in April 2015, having started out as LuaObj in 2012, and is made available under the MIT licence.

– Ben Hallstein, 2012/15
