#pragma once
#include <type_traits>
#include "conversion_base.hpp"
#include "pointer_converter.hpp"

namespace luabind {

	/*
		TODO: Remove code duplication
	*/
	namespace detail {

		struct ref_converter : pointer_converter
		{
			typedef ref_converter type;
			typedef std::false_type is_native;

			enum { consumed_args = 1 };

			template<class T>
			void apply(lua_State* L, T& ref)
			{
				if (luabind::get_back_reference(L, ref))
					return;

				make_pointee_instance(L, ref, std::false_type());
			}

			template<class T>
			T& apply(lua_State* L, by_reference<T>, int index)
			{
				assert(!lua_isnil(L, index));
				return *pointer_converter::apply(L, by_pointer<T>(), index);
			}

			template<class T>
			int match(lua_State* L, by_reference<T>, int index)
			{
				object_rep* obj = get_instance(L, index);
				if (obj == 0) return -1;

				if (obj->is_const())
					return -1;

				std::pair<void*, int> s = obj->get_instance(registered_class<T>::id);
				result = s.first;
				return s.second;
			}

			template<class T>
			void converter_postcall(lua_State*, T, int) {}
		};

		struct const_ref_converter
		{
			typedef const_ref_converter type;
			typedef std::false_type is_native;

			enum { consumed_args = 1 };

			const_ref_converter()
				: result(0)
			{}

			void* result;

			template<class T>
			void apply(lua_State* L, T const& ref)
			{
				if (luabind::get_back_reference(L, ref))
					return;

				make_pointee_instance(L, ref, std::false_type());
			}

			template<class T>
			T const& apply(lua_State*, by_const_reference<T>, int)
			{
				return *static_cast<T*>(result);
			}

			template<class T>
			int match(lua_State* L, by_const_reference<T>, int index)
			{
				object_rep* obj = get_instance(L, index);
				if (obj == 0) return -1; // if the type is not one of our own registered types, classify it as a non-match

				std::pair<void*, int> s = obj->get_instance(registered_class<T>::id);
				if (s.second >= 0 && !obj->is_const())
					s.second += 10;
				result = s.first;
				return s.second;
			}

			template<class T>
			void converter_postcall(lua_State*, by_const_reference<T>, int)
			{
			}
		};

	}

}