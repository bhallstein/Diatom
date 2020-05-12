
About LuaObj
============

LuaObj is for converting Lua tables into nested C++ objects.

- The constructor simply takes a filename and the name of the (global) object to load, returning a LuaObj representing that object.
- If the object is a table, the `descendants` map is populated recursively with the contents of the table.
    - Descendants are accessed as follows: `myObj["descendant_name"]`
- Apart from tables, LuaObj currently only supports numbers (i.e. floats), strings, bools, and nil.
	- The type is represented in the `type` property, which is of the enum type `LuaObj::Type::T`
	- Type names are: `Numeric`, `Bool`, `String`, `Table`, `Nil`
	- Creating a LuaObj on a global object of unsupported type will return a LuaObj of type Nil
	- Table members of unsupported type are skipped, i.e. not added to the descendants map at all
- The value of the object is stored in one of the following properties, depending on type:
	- `float number_value`
	- `bool bool_value`
	- `std::string str_value`
- Table keys:
	- When populating the descendants map, LuaObj turns all table keys into strings
	- The map is sorted using the Numericoid String Comparator, i.e. '10zxc' will come after '1zxc'.

See the .h for further details.

LuaObj is made available under the MIT licence.

– Ben Hallstein, 2012
