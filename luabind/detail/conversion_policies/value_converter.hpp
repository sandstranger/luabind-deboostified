#pragma once
#include <type_traits>
#include <luabind\lua_include.hpp>

namespace luabind {

	namespace detail {

		struct value_converter
		{
			typedef value_converter type;
			typedef std::false_type is_native;

			enum { consumed_args = 1 };

			value_converter()
				: result(0)
			{}

			void* result;

			template<class T>
			void apply(lua_State* L, T x)
			{
				if (luabind::get_back_reference(L, x))
					return;

				make_pointee_instance(L, std::move(x), std::true_type());
			}

			template<class T>
			T apply(lua_State*, by_value<T>, int)
			{
				return *static_cast<T*>(result);
			}

			template<class T>
			int match(lua_State* L, by_value<T>, int index)
			{
				// special case if we get nil in, try to match the holder type
				if (lua_isnil(L, index))
					return -1;

				object_rep* obj = get_instance(L, index);
				if (obj == 0) return -1;

				std::pair<void*, int> s = obj->get_instance(registered_class<T>::id);
				result = s.first;
				return s.second;
			}

			template<class T>
			void converter_postcall(lua_State*, T, int) {}
		};

	}

}