
About LuaObj
============

LuaObj is a class for fetching tables of objects of basic types from lua.

- The constructor simply takes a lua_State, returning a LuaObj representing the object at -1 on the lua stack.
- If the object is a table, the 'descendants' map is populated recursively with the contents of the table.
    - Descendants are accessed as follows: `myObj["descendant_name"]`
- Apart from tables, LuaObj currently only supports numbers (stored as floats), strings, bools, and nil.
	- The type is represented in the `type` property, which is of the enum type `LuaObj::ValueType`
	- Type names are: `NUMBER`, `BOOL`, `STRING`, `TABLE`, `NIL`
	- Creating a LuaObj on an unsupported type will return a LuaObj of type NIL
	- Table members of unsupported type are skipped, i.e. not added to the descendants map at all
- The value of the object is stored in one of the following properties, depending on type:
	- `float number_value`
	- `bool bool_value`
	- `std::string str_value`
- LuaObj turns all table keys into strings
- The descendants map is sorted by the Numericoid String Comparator, i.e. '10zxc' is sorted after '1zxc'.

See the .h for further details.

LuaObj is published under the open source MIT licence.

Ben Hallstein, 2012
