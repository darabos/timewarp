-------------------------------------------------------------------------------
-- Class.lua
-- adds basic Object-Oriented functionality
--
-- Revision History:
--  2003.11.28 youbastrd Started this, based on sample code from the Lua wiki
--             (thanks to the wiki authors!)
-------------------------------------------------------------------------------



--Creates a new class.  Optionally, this makes the returned class a subclass of
-- some existing class which was created previously with call to this function.
-- The created class will have the following functions:
-- new(), class(), superClass(), isa()
--Example:
--   Mammal = new Class(nil)
--   Dog = new Class(Mammal)
--   Cat = new Class(Mammal)
--   Fido = Dog:new()
--   print( Fido.isa(Mammal))    -- prints true
--   print( Fido.isa(Dog))       -- prints true
--   print( Fido.isa(Cat))       -- prints false
--
--@param baseClass the base class to use.  Classes with no super-classes should call
-- Class(nil)
-- function Class( baseClass )
--     local new_class = baseClass or {};
--     local class_mt = { __index = new_class };
-- 
--     function new_class:new(options)
--         local newinst = new_class;
--          if ((options) and (type(options)=="table")) then
--             for k,v in pairs(options) do
--                newinst[k] = v
--             end
--          end
-- 
--         setmetatable( newinst, class_mt );
--         return newinst;
--     end;
-- 
--     if( nil ~= baseClass ) then
--         setmetatable( new_class, { __index = baseClass } );
--     end
-- 
--     -- Implementation of additional OO properties starts here --
-- 
--     -- Return the class object of the instance
--     function new_class:class()
--         return new_class;
--     end;
-- 
--     -- Return the super class object of the instance
--     function new_class:superClass()
--         return baseClass;
--     end;
-- 
--     -- Return true if the caller is an instance of theClass
--     function new_class:isa( theClass )
--         local b_isa = false;
-- 
--         local cur_class = new_class;
-- 
--         while( ( nil ~= cur_class ) and ( false == b_isa ) ) do
--             if( cur_class == theClass ) then
--                 b_isa = true;
--             else
--                 cur_class = cur_class:superClass();
--             end;
--         end;
--
--         return b_isa;
--     end;
--
--     return new_class;
-- end


-- A new createSubclass() function --

function Class( baseClass )

    local new_class = {};
    local class_mt = { __index = new_class };

    function new_class:new(objects)
        local newinst = {};
        if (objects and type(objects)=="table") then
           newinst = objects
        end


        setmetatable( newinst, class_mt );
        return newinst;
    end;

    if( nil ~= baseClass ) then
        setmetatable( new_class, { __index = baseClass } );
    end

    -- Implementation of additional OO properties starts here --

    -- Return the class object of the instance
    function new_class:class()
        return new_class;
    end;

    -- Return the super class object of the instance
    function new_class:superClass()
        return baseClass;
    end;

    -- Return true if the caller is an instance of theClass
    function new_class:isa( theClass )
        local b_isa = false;

        local cur_class = new_class;

        while( ( nil ~= cur_class ) and ( false == b_isa ) ) do
            if( cur_class == theClass ) then
                b_isa = true;
            else
                cur_class = cur_class:superClass();
            end;
        end;

        return b_isa;
    end;

    return new_class;
end













