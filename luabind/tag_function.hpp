// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_TAG_FUNCTION_081129_HPP
#define LUABIND_TAG_FUNCTION_081129_HPP

#include <luabind/detail/meta.hpp>
#include <luabind/lua_state_fwd.hpp>

namespace luabind {

	namespace detail
	{

		struct invoke_context;
		struct function_object;

		template <class Signature, class F>
		struct tagged_function
		{
			tagged_function(F f)
				: f(f)
			{}

			F f;
		};

		template <class Signature, class F>
		Signature deduce_signature(tagged_function<Signature, F> const&, ...)
		{
			return Signature();
		}

		template <class Signature, class F, typename... PolicyInjectors>
		int invoke(
			lua_State* L, function_object const& self, invoke_context& ctx
			// std::bind operator() is nonconst... is that fixable?
			, tagged_function<Signature, F> /*const*/& tagged
			, Signature, meta::type_list<PolicyInjectors...> const& injectors)
		{
			return invoke(L, self, ctx, tagged.f, Signature(), injectors);
		}

		template <class Function>
		struct signature_from_function;


		template <typename R, typename... Args >
		struct signature_from_function<R(Args...)>
		{
			typedef meta::type_list<R, Args...> type;
		};

	} // namespace detail

	template <class Signature, class F>
	detail::tagged_function< typename detail::signature_from_function<Signature>::type, F >
		tag_function(F f)
	{
		return f;
	}

} // namespace luabind

# endif // LUABIND_TAG_FUNCTION_081129_HPP

