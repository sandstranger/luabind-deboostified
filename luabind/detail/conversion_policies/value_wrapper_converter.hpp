#pragma once
#include "conversion_base.hpp"
#include <luabind\value_wrapper.hpp>
#include <type_traits>

namespace luabind {

	namespace detail {

		template <class U>
		struct value_wrapper_converter
		{
			typedef value_wrapper_converter<U> type;
			typedef std::true_type is_native;

			enum { consumed_args = 1 };

			template<class T>
			T apply(lua_State* L, by_const_reference<T>, int index)
			{
				return T(from_stack(L, index));
			}

			template<class T>
			T apply(lua_State* L, by_value<T>, int index)
			{
				return apply(L, by_const_reference<T>(), index);
			}

			template<class T>
			static int match(lua_State* L, by_const_reference<T>, int index)
			{
				return value_wrapper_traits<T>::check(L, index)
					? (std::numeric_limits<int>::max)() / LUABIND_MAX_ARITY
					: -1;
			}

			template<class T>
			static int match(lua_State* L, by_value<T>, int index)
			{
				return match(L, by_const_reference<T>(), index);
			}

			void converter_postcall(...) {}

			template<class T>
			void apply(lua_State* interpreter, T const& value_wrapper)
			{
				value_wrapper_traits<T>::unwrap(interpreter, value_wrapper);
			}
		};

	}

}