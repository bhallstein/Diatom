# Diatom

A C++ object that stores strings, numbers, bools, or other Diatoms. These objects then trivially serialize to and from string.

Created to collect data in a video game prior to serialization.

Written in 2012, modified 2015 & 2020. Doesn't include unicode support. It's very much a hobbyist/tinkering project, not for use.

To run tests: `bash run.sh` from the `/test` directory.

```
Diatoms are single-celled algae that float freely in the ocean. Encased in transparent silica, they take a variety of incredibly beautiful (microscopic) forms, and are the only organisms to have cell walls made from glass.
```


## Types

A Diatom is a string, number (double), boolean, or nil. Or a Diatom is a **table** holding other Diatoms.

```cpp
Diatom d = 2.718;
d.is_number();    // -> true
d.number_value(); // -> 2.718
```

Diatom's defult constructor produces a table Diatom:

```cpp
Diatom d;
d.is_table();         // => true
d["my_prop"] = 7.;    // d["my_prop"] is now number Diatom with value 7.

d["my_prop"].value__number;    // => 2.718
```


## Interface

### Types:

```cpp
Diatom::Type::Number
Diatom::Type::Bool
Diatom::Type::String
Diatom::Type::Table
Diatom::Type::Nil
```

### Properties

```cpp
Type::T type;
double           value__number;
bool             value__bool;
std::string      value__string;
TableEntryVector descendants;
```

### Methods

```cpp
bool is_nil()
bool is_number()
bool is_bool()
bool is_string()
bool is_table()

Diatom operator[](const Diatom &)

template <class F>
void each(F f)
	// for a table Diatom, calls f(std::string name, Diatom entry) for each child

template <class F>
void recurse(F f)
	// for a table Diatom, recurses through the table while calling
	// f(std::string name, Diatom entry)
```


## Constructing

The default constructor creates a table Diatom.
```cpp
Diatom d_table;
```

There are constructors for each Diatom type.

```cpp
Diatom d_pi = 3.14;
Diatom d_str = "A string Diatom";
Diatom d_bool = true;

or:

Diatom d_pi{3.14};
Diatom d_str{A string Diatom};
Diatom d_bool{true};
```

Diatoms are copyable, including tables, which deep-copy:
```cpp
Diatom d1;
d1["birds"] = Diatom();
d1["birds"]["puffins"] = "Fratercula arctica";

Diatom d2 = d1;
```


## Serialization

`DiatomSerialization.h` defines methods to convert Diatoms to and from strings. The format is line by line key value pairs:

```
lemurs: 5
birds:
  blue_tits: "14"
  crows: false
  aquatic:
    penguins: 10
```



```cpp
std::string diatom__serialize(Diatom &d)
DiatomParseResult diatom__unserialize(const std::string &s)
```

`DiatomParseResult` is a struct as follows:

```cpp
  bool success;
  std::string error_string;
  Diatom d;
```

**Example:**

```cpp
std::string input =
  "lemurs: 5\n"
  "birds:\n"
  "  blue_tits: \"14\"\n"
  "  aquatic:\n"
  "    penguins: 10\n"
  "  crows: false\n";

auto animal_inventory = diatom__unserialize(input);
animal_inventory.success                     // -> true
animal_inventory.d["lemurs"].value__number   // -> 5
```

```cpp
Diatom romeo_and_juliet;
d["characters"] = Diatom();
d["characters"]["romeo"] = "montague";
d["characters"]["juliet"] = "capulet";

diatom__serialize(d);     // characters:
                          //   romeo: "montague"
													//   juliet: "capulet"
```

