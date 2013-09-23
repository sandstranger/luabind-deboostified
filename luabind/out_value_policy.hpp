// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.


#ifndef LUABIND_OUT_VALUE_POLICY_HPP_INCLUDED
#define LUABIND_OUT_VALUE_POLICY_HPP_INCLUDED

#include <luabind/config.hpp>
#include <luabind/detail/policy.hpp>    // for find_conversion_policy, etc
#include <luabind/detail/decorate_type.hpp>  // for LUABIND_DECORATE_TYPE
#include <luabind/detail/primitives.hpp>  // for by_pointer, by_reference, etc
#include <luabind/detail/typetraits.hpp>  // for is_nonconst_pointer, is_nonconst_reference, etc
#include <new>                          // for operator new

namespace luabind { namespace detail
{
	template<int N>
	struct char_array
	{
		char storage[N];
	};

	template<class U>
	char_array<sizeof(typename identity<U>::type)> indirect_sizeof_test(by_reference<U>);

	template<class U>
	char_array<sizeof(typename identity<U>::type)> indirect_sizeof_test(by_const_reference<U>);

	template<class U>
	char_array<sizeof(typename identity<U>::type)> indirect_sizeof_test(by_pointer<U>);

	template<class U>
	char_array<sizeof(typename identity<U>::type)> indirect_sizeof_test(by_const_pointer<U>);

	template<class U>
	char_array<sizeof(typename identity<U>::type)> indirect_sizeof_test(by_value<U>);

	template<class T>
	struct indirect_sizeof
	{
		static const int value = sizeof(indirect_sizeof_test(LUABIND_DECORATE_TYPE(T)));
	};
	
	template<int Size, class Policies = no_injectors>
	struct out_value_converter
	{
		enum { consumed_args = 1 };

        template<class T>
		T& apply(lua_State* L, by_reference<T>, int index)
		{
			applied_converter_policy<1, Policies, T, lua_to_cpp> converter;

#if defined(__GNUC__) && __GNUC__ >= 4
			T* storage = reinterpret_cast<T*>(m_storage);
			new (storage) T(converter.apply(L, LUABIND_DECORATE_TYPE(T), index));
			return *storage;
#else
			new (m_storage) T(converter.apply(L, LUABIND_DECORATE_TYPE(T), index));
			return *reinterpret_cast<T*>(m_storage);
#endif
		}

		template<class T>
		static int match(lua_State* L, by_reference<T>, int index)
		{
			return applied_converter_policy<1, Policies, T, lua_to_cpp >::match(L, LUABIND_DECORATE_TYPE(T), index);
		}

		template<class T>
		void converter_postcall(lua_State* L, by_reference<T>, int) 
		{
			applied_converter_policy<2,Policies,T,cpp_to_lua> converter;
#if defined(__GNUC__) && __GNUC__ >= 4
			T* storage = reinterpret_cast<T*>(m_storage);
			converter.apply(L, *storage);
			storage->~T();
#else
			converter.apply(L, *reinterpret_cast<T*>(m_storage));
			reinterpret_cast<T*>(m_storage)->~T();
#endif
		}

		template<class T>
		T* apply(lua_State* L, by_pointer<T>, int index)
		{
			applied_converter_policy<1, Policies, T, lua_to_cpp > converter;
#if defined(__GNUC__) && __GNUC__ >= 4
			T* storage = reinterpret_cast<T*>(m_storage);
			new (storage) T(converter.apply(L, LUABIND_DECORATE_TYPE(T), index));
			return storage;
#else
			new (m_storage) T(converter.apply(L, LUABIND_DECORATE_TYPE(T), index));
			return reinterpret_cast<T*>(m_storage);
#endif
		}

		template<class T>
		static int match(lua_State* L, by_pointer<T>, int index)
		{
			return applied_converter_policy<1,Policies,T,lua_to_cpp>::match(L, LUABIND_DECORATE_TYPE(T), index);
		}

		template<class T>
		void converter_postcall(lua_State* L, by_pointer<T>, int)
		{
			applied_converter_policy<2,Policies,T,cpp_to_lua> converter;
#if defined(__GNUC__) && __GNUC__ >= 4
			T* storage = reinterpret_cast<T*>(m_storage);
			converter.apply(L, *storage);
			storage->~T();
#else
			converter.apply(L, *reinterpret_cast<T*>(m_storage));
			reinterpret_cast<T*>(m_storage)->~T();
#endif
		}

		char m_storage[Size];
	};

	template<class Policies = no_injectors>
	struct out_value_policy : conversion_policy<>
	{
		struct only_accepts_nonconst_references_or_pointers {};
		struct can_only_convert_from_lua_to_cpp {};

		template<class T, class Direction>
		struct apply
		{
			static_assert(std::is_same< Direction, lua_to_cpp >::value, "Can only convert from lua to cpp");
			static_assert(meta::or_< is_nonconst_reference<T>, is_nonconst_pointer<T> >::value, "Only accepts non const references or pointers");

			typedef out_value_converter<indirect_sizeof<T>::value, Policies> type;
		};
	};

	template<int Size, class Policies = no_injectors>
	struct pure_out_value_converter
	{
		enum { consumed_args = 0 };

        template<class T>
		T& apply(lua_State*, by_reference<T>, int)
		{
#if defined(__GNUC__) && __GNUC__ >= 4
			T* storage = reinterpret_cast<T*>(m_storage);
			new (storage) T();
			return *storage;
#else
			new (m_storage) T();
			return *reinterpret_cast<T*>(m_storage);
#endif
		}

		template<class T>
		static int match(lua_State*, by_reference<T>, int)
		{
			return 0;
		}

		template<class T>
		void converter_postcall(lua_State* L, by_reference<T>, int) 
		{
			applied_converter_policy<1,Policies,T,cpp_to_lua> converter;

#if defined(__GNUC__) && __GNUC__ >= 4
			T* storage = reinterpret_cast<T*>(m_storage);
			converter.apply(L, *storage);
			storage->~T();
#else
			converter.apply(L, *reinterpret_cast<T*>(m_storage));
			reinterpret_cast<T*>(m_storage)->~T();
#endif
		}

		template<class T>
		T* apply(lua_State*, by_pointer<T>, int)
		{
#if defined(__GNUC__) && __GNUC__ >= 4
			T* storage = reinterpret_cast<T*>(m_storage);
			new (storage) T();
			return storage;
#else
			new (m_storage) T();
			return reinterpret_cast<T*>(m_storage);
#endif
		}

		template<class T>
		static int match(lua_State*, by_pointer<T>, int)
		{
			return 0;
		}

		template<class T>
		void converter_postcall(lua_State* L, by_pointer<T>, int) 
		{
			applied_converter_policy<1,Policies,T,cpp_to_lua> converter;
#if defined(__GNUC__) && __GNUC__ >= 4
			T* storage = reinterpret_cast<T*>(m_storage);
			converter.apply(L, *storage);
			storage->~T();
#else
			converter.apply(L, *reinterpret_cast<T*>(m_storage));
			reinterpret_cast<T*>(m_storage)->~T();
#endif
		}

		//std::aligned_storage< Size, Size > m_storage;
		char m_storage[Size];
	};

	template<class Policies = no_injectors>
	struct pure_out_value_policy : conversion_policy<false>
	{
		struct only_accepts_nonconst_references_or_pointers {};
		struct can_only_convert_from_lua_to_cpp {};

		template<class T, class Direction>
		struct apply
		{
			static_assert(std::is_same< Direction, lua_to_cpp >::value, "Can only convert from lua to cpp");
			static_assert(meta::or_< is_nonconst_reference<T>, is_nonconst_pointer<T> >::value, "Only accepts non const references or pointers");

			typedef pure_out_value_converter<indirect_sizeof<T>::value, Policies> type;
		};
	};
	
}}

namespace luabind
{
	template<int N>
	meta::type_list< converter_policy_injector< N, detail::out_value_policy< > > > 
	out_value(LUABIND_PLACEHOLDER_ARG(N)) 
	{ 
		return meta::type_list < converter_policy_injector< N, detail::out_value_policy< > >();
	}

	template<int N, class Policies>
	meta::type_list < converter_policy_injector< N, detail::out_value_policy< Policies > > >
	out_value(LUABIND_PLACEHOLDER_ARG(N), const Policies&) 
	{ 
		return meta::type_list < converter_policy_injector< N, detail::out_value_policy< Policies > >();
	}

	template<int N>
	meta::type_list< converter_policy_injector< N, detail::pure_out_value_policy< > > >
	pure_out_value(LUABIND_PLACEHOLDER_ARG(N)) 
	{ 
		return meta::type_list< converter_policy_injector< N, detail::pure_out_value_policy< > > >();
	}

	template<int N, class Policies>
	meta::type_list< converter_policy_injector< N, detail::pure_out_value_policy<Policies> > >
	pure_out_value(LUABIND_PLACEHOLDER_ARG(N), const Policies&) 
	{ 
		return meta::type_list< converter_policy_injector< N, detail::pure_out_value_policy<Policies> > >();
	}
}

#endif // LUABIND_OUT_VALUE_POLICY_HPP_INCLUDED

