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

#ifndef LUABIND_CALL_FUNCTION_HPP_INCLUDED
#define LUABIND_CALL_FUNCTION_HPP_INCLUDED

#include <luabind/config.hpp>

#include <luabind/error.hpp>
#include <luabind/detail/convert_to_lua.hpp>
#include <luabind/detail/pcall.hpp>
#include <luabind/detail/call_shared.hpp>

namespace luabind
{
	namespace detail {

		template<typename Ret, typename PolicyList, typename... Args, unsigned int... Indices, typename Fn>
		void call_function_impl(lua_State* L, int m_params, Fn fn, std::true_type, meta::index_list<Indices...>, Args&&... args)
		{
			int top = lua_gettop(L);
			meta::expand_calls_hack( (
				applied_converter_policy<Indices, PolicyList, typename unwrapped<Args>::type, cpp_to_lua>().apply(L, unwrapped<Args>::get(std::forward<Args>(args))),0)...
				);

			if (fn(L, sizeof...(Args), 0)) {
				assert(lua_gettop(L) == top - m_params + 1);
				call_error(L);
			}
			// pops the return values from the function call
			stack_pop pop(L, lua_gettop(L) - top + m_params);
		}

		template<typename Ret, typename PolicyList, typename... Args, unsigned int... Indices, typename Fn>
		Ret call_function_impl(lua_State* L, int m_params, Fn fn, std::false_type, meta::index_list<Indices...>, Args&&... args)
		{
			int top = lua_gettop(L);
			meta::expand_calls_hack( (
				applied_converter_policy<Indices, PolicyList, typename unwrapped<Args>::type, cpp_to_lua>().apply(L, unwrapped<Args>::get(std::forward<Args>(args))), 0)...
				);

			if (fn(L, sizeof...(Args), 1)) {
				assert(lua_gettop(L) == top - m_params + 1);
				call_error(L);
			}
			// pops the return values from the function call
			stack_pop pop(L, lua_gettop(L) - top + m_params);

			applied_converter_policy<0, PolicyList, Ret, lua_to_cpp> converter;
			if (converter.match(L, decorated_type<Ret>(), -1) < 0) {
				cast_error<Ret>(L);
			}

			return converter.apply(L, decorated_type<Ret>(), -1);
		}

	}

	template<class R, typename PolicyList = no_injectors, typename... Args>
	R call_function(lua_State* L, const char* name, Args&&... args )
	{
		assert(name && "luabind::call_function() expects a function name");
		lua_getglobal(L, name);
		return detail::call_function_impl<R, PolicyList>(L, 1, &detail::pcall, std::is_void<R>(), meta::index_range<1, sizeof...(Args) +1>(), std::forward<Args>(args)...);
	}

	template<class R, typename PolicyList = no_injectors, typename... Args>
	R call_function(luabind::object const& obj, Args&&... args)
	{
		obj.push(obj.interpreter());
		return detail::call_function_impl<R, PolicyList>(obj.interpreter(), 1, &detail::pcall, std::is_void<R>(), meta::index_range<1, sizeof...(Args) +1>(), std::forward<Args>(args)...);
	}

	template<class R, typename PolicyList = no_injectors, typename... Args>
	R resume_function(lua_State* L, const char* name, Args&&... args)
	{
		assert(name && "luabind::resume_function() expects a function name");
		lua_getglobal(L, name);
		return detail::call_function_impl<R, PolicyList>(L, 1, &detail::resume_impl, std::is_void<R>(), meta::index_range<1, sizeof...(Args) +1>(), std::forward<Args>(args)...);
	}

	template<class R, typename PolicyList = no_injectors, typename... Args>
	R resume_function(luabind::object const& obj, Args&&... args)
	{
		obj.push(obj.interpreter());
		return detail::call_function_impl<R, PolicyList>(obj.interpreter(), 1, &detail::resume_impl, std::is_void<R>(), meta::index_range<1, sizeof...(Args) +1>(), std::forward<Args>(args)...);
	}

	template<class R, typename PolicyList = no_injectors, typename... Args>
	R resume(lua_State* L, Args&&... args)
	{
		return detail::call_function_impl<R, PolicyList>(L, 0, &detail::resume_impl, std::is_void<R>(), meta::index_range<1, sizeof...(Args) +1>(), std::forward<Args>(args)...);
	}

}

#endif // LUABIND_CALL_FUNCTION_HPP_INCLUDED

