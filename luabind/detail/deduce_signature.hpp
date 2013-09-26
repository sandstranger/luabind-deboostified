// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# ifndef LUABIND_DEDUCE_SIGNATURE_080911_HPP
# define LUABIND_DEDUCE_SIGNATURE_080911_HPP

#include <luabind/detail/meta.hpp>
#include <luabind/detail/most_derived.hpp>

namespace luabind { namespace detail {

template< typename, typename > struct tagged_function;



template< typename T, typename WrappedType = detail::null_type >
struct call_types;

template< typename R, typename... Args, typename WrappedType >
struct call_types < R(*)(Args...), WrappedType >
{
	typedef meta::type_list< R, Args... > signature_type;
};

template< typename R, typename Class, typename... Args >
struct call_types < R(Class::*)(Args...), detail::null_type >
{
	typedef meta::type_list< R, Class&, Args... > signature_type;
};

template< typename R, typename Class, typename... Args >
struct call_types < R(Class::*)(Args...) const, detail::null_type >
{
	typedef meta::type_list< R, Class const&, Args... > signature_type;
};

template< typename R, typename Class, typename... Args, class WrappedType >
struct call_types < R(Class::*)(Args...), WrappedType >
{
	typedef meta::type_list< R, typename most_derived<Class, WrappedType>::type&, Args... > signature_type;
};

template< typename R, typename Class, typename... Args, class WrappedType >
struct call_types < R(Class::*)(Args...) const, WrappedType >
{
	typedef meta::type_list< R, typename most_derived<Class, WrappedType>::type const&, Args... > signature_type;
};

template< typename Signature, typename F, class WrappedType >
struct call_types< tagged_function< Signature, F >, WrappedType >
{
	typedef Signature signature_type;
};

/* Apply policy injectors to a policy list */
template< typename PolicyList, typename InjectorList >
struct apply_injectors;

template< typename PolicyList, unsigned int Injector0_Index, typename Injector0_Type, typename... Injectors >
struct apply_injectors< PolicyList, meta::type_list< converter_policy_injector<Injector0_Index, Injector0_Type>, Injectors... > >
{
	typedef typename apply_injectors< typename meta::replace< PolicyList, Injector0_Index, Injector0_Type >::type, meta::type_list< Injectors... > >::type type;
};

template< typename PolicyList >
struct apply_injectors< PolicyList, meta::type_list< > >
{
	typedef PolicyList type;
};

/*
	Need to create an overload that deduces the signature of a no-argument operator() ?
	Some client code inside luabind creates a custom typelist, because deduce_signature cannot deduce the signature of functors
*/

}} // namespace luabind::detail

# endif // LUABIND_DEDUCE_SIGNATURE_080911_HPP

