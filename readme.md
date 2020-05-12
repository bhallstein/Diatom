
# About LuaObj

LuaObj is a class which recursively converts Lua tables into nested C++ objects.

## Creating

The constructor simply takes a filename and the name of the (global) object to load, returning a LuaObj representing that object:

    LuaObj myObj("file.lua", "a_table");

- If the object is a table, the `descendants` map is populated recursively with the contents of the table.
- If the object is a supported simple type, it is represented in the appropriate LuaObj property.

Alternatively, you can construct a LuaObj from a Lua state that you manage yourself. You just need to push the table you want to load to position -1 on the stack:

    lua_State *L;
    if (!LuaObj::loadLuaFile("file.lua", &L))
    	; // oh dear
    
    lua_getglobal(L, "a_table");
    LuaObj myObj(L);
    lua_pop(L, 1);
    
    lua_close(L);


## Types & Values

A LuaObj may represent a table containing other values, or a number, string, bool, or nil. The type is stored in the object’s `type` property, and is one of the enum `LuaObj::Type::T` values, `Numeric`, `Bool`, `String`, `Table`, or `_Nil`.

The represented value itself is stored in another property – depending on type, `double number_value`, `bool bool_value`, or `std::string str_value`.

    if (myObj.type == LuaObj::Type::Table)
        ; // iterate its descendants, perhaps
    
    if (myObj.type == LuaObj::Type::Numeric)
        double f = myObj.number_value;


## Accessing Descendants

Descendants are accessed using the subscript operator:

    `LuaObj a_descendant = myObj["descendant_name"];`


Table members of unsupported type are skipped, i.e. not added to the descendants map at all.

On descendant ordering:

- When populating the descendants map, LuaObj turns all table keys into strings
- The map is sorted using the Numericoid String Comparator, i.e. '10zxc' will come after '1zxc'.


## Kthx

See the .h for further details.

LuaObj is made available under the MIT licence.

– Ben Hallstein, 2012
