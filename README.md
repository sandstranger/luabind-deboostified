luabind
=======

Create Lua bindings for your C++ code easily - my improvements
- Variadic templates.
- Got rid of the arrays created in invoke.
- All boost removed (except if the nothrow policy shall be used).
- No variadic simulations left.
- No backward compatibility to any old or faulty (MS) compilers.

- Important: This is not drop in replacable. I removed the quirky auto detection mode on the 4 class_ template parameters, so you have to explicitly say class_&lt;MyClass, bases&lt;Any Number of bases>, WrapperType, HolderType> .... other than that I think it is pretty compatible.
- Note: This is intended for VC++2013 RC currently and lots of fixups have to be done. (#pragma once etc.)
