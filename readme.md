
# About LuaObj

LuaObj is a class which recursively converts Lua tables into nested C++ objects.

## Creating

The constructor simply takes a filename and the name of the (global) object to load, returning a LuaObj representing that object:

    LuaObj myObj("file.lua", "a_table");

- If the object is a table, the `descendants` map is populated recursively with the contents of the table.
- If the object is a supported simple type, it is represented in the appropriate LuaObj property.


## Types & Values

Apart from tables, LuaObj supports numbers (i.e. floats), strings, bools, and nil.

- The type is represented in the `type` property, which is of the enum type `LuaObj::Type::T`
	- Type names are: `Numeric`, `Bool`, `String`, `Table`, `Nil`
- The value of the object is stored in one of the following properties, depending on type:
	- `float number_value`
	- `bool bool_value`
	- `std::string str_value`

    if (myObj.type == LuaObj::Type::Table)
		// iterate its descendants, perhaps
		
	if (myObj.type == LuaObj::Type::Numeric)
		float f = myObj.number_value;


## Accessing Descendants

Descendants are accessed using the subscript operator:

    `LuaObj a_descendant = myObj["descendant_name"];`


- Table members of unsupported type are skipped, i.e. not added to the descendants map at all
- Table keys:
	- When populating the descendants map, LuaObj turns all table keys into strings
	- The map is sorted using the Numericoid String Comparator, i.e. '10zxc' will come after '1zxc'.

See the .h for further details.

LuaObj is made available under the MIT licence.

– Ben Hallstein, 2012
