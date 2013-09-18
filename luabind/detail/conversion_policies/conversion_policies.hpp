#pragma once

#include <luabind/detail/typetraits.hpp>
#include <luabind/detail/meta.hpp>
#include "conversion_base.hpp"
#include "enum_converter.hpp"
#include "pointer_converter.hpp"
#include "reference_converter.hpp"
#include "value_converter.hpp"
#include "value_wrapper_converter.hpp"
#include "native_converter.hpp"

namespace luabind {

	template <>
	struct default_converter<lua_State*>
	{
		enum { consumed_args = 0 };

		template <class U>
		lua_State* apply(lua_State* L, U, int index)
		{
			return L;
		}

		template <class U>
		static int match(lua_State*, U, int index)
		{
			return 0;
		}

		template <class U>
		void converter_postcall(lua_State*, U, int) {}
	};

	namespace detail {

		template< class T >
		struct default_converter_generator
			: public meta::select_ <
				meta::case_< is_value_wrapper_arg2<T>, value_wrapper_converter<T> >,
				meta::case_< std::is_enum<typename std::remove_reference<T>::type>, enum_converter >,
				meta::case_< is_nonconst_pointer<T>, pointer_converter >,
				meta::case_< is_const_pointer<T>, const_pointer_converter >,
				meta::case_< is_nonconst_reference<T>, ref_converter >,
				meta::case_< is_const_reference<T>, const_ref_converter >,
				meta::default_< value_converter >
			> ::type
		{
		};
	
	}

	template <class T, class Enable>
	struct default_converter
		: detail::default_converter_generator<T>::type
	{};

}