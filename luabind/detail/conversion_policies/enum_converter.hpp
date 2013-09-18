#pragma once
#include <type_traits>
#include <luabind\detail\typetraits.hpp>
#include "conversion_base.hpp"

namespace luabind {

	namespace detail {

		struct enum_converter
		{
			typedef enum_converter type;
			typedef std::false_type is_native;

			enum { consumed_args = 1 };

			void apply(lua_State* L, int val)
			{
				lua_pushnumber(L, val);
			}

			template<class T>
			T apply(lua_State* L, by_value<T>, int index)
			{
				return static_cast<T>(static_cast<int>(lua_tonumber(L, index)));
			}

			template<class T>
			static int match(lua_State* L, by_value<T>, int index)
			{
				if (lua_isnumber(L, index)) return 0; else return -1;
			}

			template<class T>
			T apply(lua_State* L, by_const_reference<T>, int index)
			{
				return static_cast<T>(static_cast<int>(lua_tonumber(L, index)));
			}

			template<class T>
			static int match(lua_State* L, by_const_reference<T>, int index)
			{
				if (lua_isnumber(L, index)) return 0; else return -1;
			}

			template<class T>
			void converter_postcall(lua_State*, T, int) {}
		};

	}

}