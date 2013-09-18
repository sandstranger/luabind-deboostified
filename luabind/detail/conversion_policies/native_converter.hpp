#pragma once
#include "conversion_base.hpp"
#include <type_traits>
#include <boost/call_traits.hpp>
#include <luabind\lua_include.hpp>
#include <string>

#if LUA_VERSION_NUM < 502
# define lua_rawlen lua_objlen
#endif

namespace luabind {

	template <class T, class Derived = default_converter<T> >
	struct native_converter_base
	{
		typedef std::true_type is_native;
		typedef typename boost::call_traits<T>::value_type value_type;
		typedef typename boost::call_traits<T>::param_type param_type;

		enum { consumed_args = 1 };
		
		template <class U>
		void converter_postcall(lua_State*, U const&, int)
		{}

		int match(lua_State* L, detail::by_value<T>, int index)
		{
			return Derived::compute_score(L, index);
		}

		int match(lua_State* L, detail::by_value<T const>, int index)
		{
			return Derived::compute_score(L, index);
		}


		int match(lua_State* L, detail::by_const_reference<T>, int index)
		{
			return Derived::compute_score(L, index);
		}

		value_type apply(lua_State* L, detail::by_value<T>, int index)
		{
			return derived().from(L, index);
		}

		value_type apply(lua_State* L, detail::by_value<T const>, int index)
		{
			return derived().from(L, index);
		}

		value_type apply(lua_State* L, detail::by_const_reference<T>, int index)
		{
			return derived().from(L, index);
		}

		void apply(lua_State* L, param_type value)
		{
			derived().to(L, value);
		}

		Derived& derived()
		{
			return static_cast<Derived&>(*this);
		}
	};

	template <typename QualifiedT>
	struct integer_converter
		: native_converter_base<typename std::remove_reference<typename std::remove_const<QualifiedT>::type>::type>
	{
		typedef typename std::remove_reference<typename std::remove_const<QualifiedT>::type>::type T;
		typedef typename native_converter_base<T>::param_type param_type;
		typedef typename native_converter_base<T>::value_type value_type;

		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
		}

		static value_type from(lua_State* L, int index)
		{
			return static_cast<T>(lua_tointeger(L, index));
		}

		void to(lua_State* L, param_type value)
		{
			lua_pushinteger(L, static_cast<lua_Integer>(value));
		}
	};

	template <typename QualifiedT>
	struct number_converter
		: native_converter_base<typename std::remove_reference<typename std::remove_const<QualifiedT>::type>::type>
	{
		typedef typename std::remove_reference<typename std::remove_const<QualifiedT>::type>::type T;
		typedef typename native_converter_base<T>::param_type param_type;
		typedef typename native_converter_base<T>::value_type value_type;

		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
		}

		static value_type from(lua_State* L, int index)
		{
			return static_cast<T>(lua_tonumber(L, index));
		}

		static void to(lua_State* L, param_type value)
		{
			lua_pushnumber(L, static_cast<lua_Number>(value));
		}
	};

	template <>
	struct default_converter<bool>
		: native_converter_base<bool>
	{
		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TBOOLEAN ? 0 : -1;
		}

		static bool from(lua_State* L, int index)
		{
			return lua_toboolean(L, index) == 1;
		}

		static void to(lua_State* L, bool value)
		{
			lua_pushboolean(L, value);
		}
	};


	template <>
	struct default_converter<bool const>
		: default_converter<bool>
	{};

	template <>
	struct default_converter<bool const&>
		: default_converter<bool>
	{};

	template <>
	struct default_converter<std::string>
		: native_converter_base<std::string>
	{
		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
		}

		static std::string from(lua_State* L, int index)
		{
			return std::string(lua_tostring(L, index), lua_rawlen(L, index));
		}

		static void to(lua_State* L, std::string const& value)
		{
			lua_pushlstring(L, value.data(), value.size());
		}
	};

	template <>
	struct default_converter<std::string const>
		: default_converter<std::string>
	{};

	template <>
	struct default_converter<std::string const&>
		: default_converter<std::string>
	{};

	template <>
	struct default_converter<char const*>
	{
		typedef boost::mpl::true_ is_native;

		enum { consumed_args = 1 };

		template <class U>
		static int match(lua_State* L, U, int index)
		{
			int type = lua_type(L, index);
			return (type == LUA_TSTRING || type == LUA_TNIL) ? 0 : -1;
		}

		template <class U>
		static char const* apply(lua_State* L, U, int index)
		{
			return lua_tostring(L, index);
		}

		static void apply(lua_State* L, char const* str)
		{
			lua_pushstring(L, str);
		}

		template <class U>
		void converter_postcall(lua_State*, U, int)
		{}
	};

	template <>
	struct default_converter<const char* const>
		: default_converter<char const*>
	{};

	template <>
	struct default_converter<char*>
		: default_converter<char const*>
	{};

	template <std::size_t N>
	struct default_converter<char const[N]>
		: default_converter<char const*>
	{};

	template <std::size_t N>
	struct default_converter<char[N]>
		: default_converter<char const*>
	{};

	// TODO: is_integral doesn't match float?
	template <typename T>
	struct default_converter < T, typename std::enable_if< std::is_integral<T>::value >::type >
		: integer_converter<T> 
	{
	};

	// template partial specialization for those types that boost knows to be
	// floating-point
	template <typename T>
	struct default_converter < T, typename std::enable_if< std::is_floating_point<T>::value >::type >
		: number_converter<T>
	{
	};

}

#if LUA_VERSION_NUM < 502
# undef lua_rawlen
#endif