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


#ifndef LUABIND_POLICY_HPP_INCLUDED
#define LUABIND_POLICY_HPP_INCLUDED

// TODO: Move all of this into conversion_policies.hpp?

#include <luabind/config.hpp>

#include <typeinfo>
#include <type_traits>
#include <string>
#include <memory>

#include <luabind/weak_ref.hpp>
#include "conversion_policies\conversion_policies.hpp"

#include "meta.hpp"

#if LUA_VERSION_NUM < 502
# define lua_rawlen lua_objlen
#endif

namespace luabind
{
	 class weak_ref;	// TODO: ???
}

namespace luabind
{

	typedef meta::type_list< > no_injectors;

	template< unsigned int Index, typename T >
	struct converter_policy_injector
	{
		enum { has_postcall = std::is_convertible<T, detail::converter_policy_has_postcall_tag >::value };
	};

	template< typename T >
	struct call_policy_injector
	{};

	// *********** converter for lua_State * arguments - consuming no explicit args *****************

	namespace detail
	{

		struct default_policy : converter_policy_tag
		{
			BOOST_STATIC_CONSTANT(bool, has_arg = true);

			template<class T>
			static void precall(lua_State*, T, int) {}

			template<class T, class Direction>
			struct apply
			{
				typedef default_converter<T> type;
			};
		};

		template<class T>
		struct is_primitive
		  : default_converter<T>::is_native
		{};

		/*
			get_converter_policy
			Finds policy injector for argument with index N or default_policy
		*/

		template< unsigned int Index, typename PoliciesList >
		struct get_converter_policy;

		template< unsigned int Index, typename Policy0, typename... Policies >
		struct get_converter_policy< Index, meta::type_list< Policy0, Policies... > >
		{
			typedef typename get_converter_policy< Index, meta::type_list< Policies... > >::type type;
		};

		template< unsigned int Index, typename ConverterPolicy, typename... Policies >
		struct get_converter_policy< Index, meta::type_list< converter_policy_injector< Index, ConverterPolicy >, Policies... > >
		{
			typedef ConverterPolicy type;
		};

		template< unsigned int Index >
		struct get_converter_policy< Index, meta::type_list< > >
		{
			typedef default_policy type;
		};

		template< typename ConverterPolicy, typename Type, typename Direction >
		struct apply_converter_policy {
			typedef typename ConverterPolicy::template apply<Type, Direction>::type type;
		};


		/*
			call_policies
		*/

		// Makes calls to precall and postcall members of call policies in a policy list.
		template< typename PolicyList >
		struct call_policies;

		template< typename CallPolicy0, typename... Policies >
		struct call_policies< meta::type_list< call_policy_injector< CallPolicy0 >, Policies... > > {
			static void apply_precall(lua_State* L, const index_map& i) {
				CallPolicy0::precall(L, i);
				call_policies< meta::type_list< Policies... > >::apply_precall(L, i);
			}
			static void apply_postcall(lua_State* L, const index_map& i) {
				CallPolicy0::postcall(L, i);
				call_policies< meta::type_list< Policies... > >::apply_postcall(L, i);
			}
		};

		template< unsigned int Index, typename Policy0, typename... Policies >
		struct call_policies< meta::type_list< converter_policy_injector< Index, Policy0 >, Policies... > > {
			static void apply_precall(lua_State* L, const index_map& i) {
				call_policies< meta::type_list< Policies... > >::apply_precall(L, i);
			}
			static void apply_postcall(lua_State* L, const index_map& i) {
				//Policy0::converter_postcall(L, i[Index]);
				Policy0::postcall(L, i);
				call_policies< meta::type_list< Policies... > >::apply_postcall(L, i);
			}
		};

		template< >
		struct call_policies< meta::type_list< > > {
			static void apply_precall(lua_State* L, const index_map& i) {
			}
			static void apply_postcall(lua_State* L, const index_map& i) {
			}
		};

		template< typename List, class Sought >
		struct has_call_policy : public meta::contains< List, call_policy_injector< Sought > >
		{
		};

		/*
			make_default_policy_list
		*/
		template< typename Signature >
		struct make_default_converter_list;

		template< typename Signature, typename... Converters >
		struct make_default_converter_list_trailing;

		template< typename HeadElement, typename... TrailingElements, typename... Converters >
		struct make_default_converter_list_trailing< meta::type_list< HeadElement, TrailingElements... >, Converters... >
		{
			typedef typename default_policy::apply<HeadElement, lua_to_cpp>::type this_converter;
			typedef typename make_default_converter_list_trailing< meta::type_list< TrailingElements... >, Converters..., this_converter >::type type;
		};

		template< typename... Converters >
		struct make_default_converter_list_trailing< meta::type_list< >, Converters... >
		{
			typedef meta::type_list< Converters... > type;
		};

		template< typename SignatureElement0, typename... SignatureElements >
		struct make_default_converter_list< meta::type_list< SignatureElement0, SignatureElements... > >
		{
			typedef typename default_policy::apply< SignatureElement0, cpp_to_lua >::type this_converter;
			typedef typename make_default_converter_list_trailing< meta::type_list< SignatureElements... >, this_converter >::type type;

			//typedef meta::type_list< 
			//	typename default_policy::apply< SignatureElement0, cpp_to_lua >::type,
			//    typename default_policy::apply< SignatureElements, lua_to_cpp >::type...	// There seems to be a serious bug here with the parameter pack expansion of msvc++?!
			//> type;			
		};

	}
} // namespace luabind::detail


namespace luabind { namespace
{
  static meta::index<0> return_value;
  static meta::index<0> result;
  static meta::index<1> _1;
  static meta::index<2> _2;
  static meta::index<3> _3;

# define LUABIND_PLACEHOLDER_ARG(N) meta::index<N>
}}

#endif // LUABIND_POLICY_HPP_INCLUDED

