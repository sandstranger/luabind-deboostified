// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_CALL2_080911_HPP
#define LUABIND_CALL2_080911_HPP

#include <luabind/config.hpp>
#include <typeinfo>
#include <luabind/detail/meta.hpp>
#include <luabind/detail/policy.hpp>
#include <luabind/yield_policy.hpp>

namespace luabind {
	namespace detail {

		struct invoke_context;

		struct LUABIND_API function_object
		{
			function_object(lua_CFunction entry)
				: entry(entry)
				, next(0)
			{}

			virtual ~function_object()
			{}

			virtual int call( lua_State* L, invoke_context& ctx ) /* const */ = 0;
			virtual void format_signature( lua_State* L, char const* function ) const = 0;

			lua_CFunction entry;
			std::string name;
			function_object* next;
			object keepalive;
		};

		struct LUABIND_API invoke_context
		{
			invoke_context()
				: best_score((std::numeric_limits<int>::max)())
				, candidate_index(0)
			{}

			operator bool() const
			{
				return candidate_index == 1;
			}

			void format_error(lua_State* L, function_object const* overloads) const;

			int best_score;
			function_object const* candidates[10];
			int candidate_index;
		};

		namespace call_detail_new {

			template< typename ConsumedList, unsigned int CurrentSum, unsigned int... StackIndices >
			struct compute_stack_indices;

			template< unsigned int Consumed0, unsigned int... Consumeds, unsigned int CurrentSum, unsigned int... StackIndices >
			struct compute_stack_indices< meta::index_list< Consumed0, Consumeds... >, CurrentSum, StackIndices... >
			{
				typedef typename compute_stack_indices< meta::index_list< Consumeds... >, CurrentSum + Consumed0, StackIndices..., CurrentSum >::type type;
			};

			template< unsigned int CurrentSum, unsigned int... StackIndices >
			struct compute_stack_indices< meta::index_list< >, CurrentSum, StackIndices... >
			{
				typedef meta::index_list< StackIndices... > type;
			};

			template< typename Foo >
			struct FooFoo {	// Foo!
				enum { consumed_args = Foo::consumed_args };
			};

			template< typename... ArgumentConverters >
			struct compute_invoke_values {
				using consumed_list    = meta::index_list< FooFoo<ArgumentConverters>::consumed_args... >;
				using stack_index_list = typename compute_stack_indices< consumed_list, 1 >::type;
				enum { arity = meta::sum<consumed_list>::value };
			};

			template< typename PolicyList, typename StackIndexList >
			struct policy_list_postcall;

			template< typename Policy0, typename... Policies, typename StackIndexList >
			struct policy_list_postcall< meta::type_list< call_policy_injector<Policy0>, Policies... >, StackIndexList > {
				static void postcall(lua_State* L, int results) {
					Policy0::postcall(L, results, StackIndexList());
					policy_list_postcall< meta::type_list< Policies... >, StackIndexList >::postcall(L,results);
				}
			};

			template< typename ConverterPolicy, typename StackIndexList, bool has_postcall >
			struct converter_policy_postcall {
				static void postcall(lua_State* L, int results) {
					ConverterPolicy::postcall(L, results, StackIndexList());
				}
			};

			template< typename ConverterPolicy, typename StackIndexList >
			struct converter_policy_postcall< ConverterPolicy, StackIndexList, false > {
				static void postcall(lua_State* L, int results) {
				}
			};

			template< unsigned int Index, typename Policy, typename... Policies, typename StackIndexList >
			struct policy_list_postcall< meta::type_list< converter_policy_injector< Index, Policy >, Policies... >, StackIndexList > {
				static void postcall(lua_State* L,int results) {
					converter_policy_postcall < Policy, StackIndexList, converter_policy_injector< Index, Policy >::has_postcall >::postcall(L, results);
					policy_list_postcall< meta::type_list< Policies... >, StackIndexList >::postcall(L, results);
				}
			};

			template< typename StackIndexList >
			struct policy_list_postcall< meta::type_list< >, StackIndexList > {
				static void postcall(lua_State* L, int results) {}
			};

		}
		
		inline int match_deferred( lua_State* L, meta::index_list<>, meta::type_list<> )
		{
			return 0;
		}

		template< unsigned int StackIndex0, unsigned int... StackIndices,
				  typename ArgumentType0, typename... ArgumentTypes,
				  typename ArgumentConverter0, typename... ArgumentConverters >
		int match_deferred( lua_State* L,
			meta::index_list< StackIndex0, StackIndices... >,
			meta::type_list< ArgumentType0, ArgumentTypes... >,
			ArgumentConverter0& converter0, ArgumentConverters&... converters
			)
		{
			const int this_match  = converter0.match(L, decorated_type<ArgumentType0>(), StackIndex0);
			const int other_match = match_deferred(L, meta::index_list<StackIndices...>(), meta::type_list<ArgumentTypes...>(), converters...);
			return (this_match >= 0 && other_match >= 0) ? this_match + other_match : -1;
		}


		template< typename T, bool isvoid, bool memfun = std::is_member_function_pointer<T>::value > struct do_call_struct;

		template< typename T >
		struct do_call_struct< T, true, true /*memfun*/> {
			template< typename F, typename ArgumentType0, typename... ArgumentTypes, unsigned int StackIndex0, unsigned int... StackIndices, typename ReturnConverter, typename Argument0Converter, typename... ArgumentConverters >
			static void do_call(lua_State* L, F& f,
						 meta::index_list<StackIndex0, StackIndices...>, meta::type_list<ArgumentType0, ArgumentTypes...>,
						 ReturnConverter& result_converter, Argument0Converter& arg0_converter, ArgumentConverters&... arg_converters
						 )
			{
				
				((arg0_converter.apply(L, decorated_type<ArgumentType0>(), StackIndex0)).*f)(
					arg_converters.apply(L, decorated_type<ArgumentTypes>(), StackIndices)...
				);
					
			}
		};

		template< typename T >
		struct do_call_struct< T, false, true /*memfun*/> {
			template< typename F, typename ArgumentType0, typename... ArgumentTypes, unsigned int StackIndex0, unsigned int... StackIndices, typename ReturnConverter, typename Argument0Converter, typename... ArgumentConverters >
			static void do_call(lua_State* L, F& f,
				meta::index_list<StackIndex0, StackIndices...>, meta::type_list<ArgumentType0, ArgumentTypes...>,
				ReturnConverter& result_converter, Argument0Converter& arg0_converter, ArgumentConverters&... arg_converters
				)
			{
				result_converter.apply(L,
					((arg0_converter.apply(L, decorated_type<ArgumentType0>(), StackIndex0)).*f)(
					arg_converters.apply(L, decorated_type<ArgumentTypes>(), StackIndices)...
					)
					);
			}
		};


		template< typename T >
		struct do_call_struct< T, true, false > {
			template<
				typename F,
				typename... ArgumentTypes, unsigned int... StackIndices,
				typename ReturnConverter, typename... ArgumentConverters
			>
			static void do_call(lua_State* L, F& f,
			meta::index_list<StackIndices...>, meta::type_list<ArgumentTypes...>,
			ReturnConverter& result_converter, ArgumentConverters&... arg_converters)
			{
				f(arg_converters.apply(L, decorated_type<ArgumentTypes>(), StackIndices)...);
			}
		};

		template< typename T >
		struct do_call_struct< T, false, false > {
			template<
				typename F,
				typename... ArgumentTypes, unsigned int... StackIndices,
				typename ReturnConverter, typename... ArgumentConverters
			>
			static void do_call(lua_State* L, F& f,
			meta::index_list<StackIndices...>, meta::type_list<ArgumentTypes...>,
			ReturnConverter& result_converter, ArgumentConverters&... arg_converters)
			{
				result_converter.apply(L,
					f(arg_converters.apply(L, decorated_type<ArgumentTypes>(), StackIndices)...)
					);
			}
		};

		template< typename F, typename ReturnType, typename... Arguments,
				  typename ReturnConverter, typename... ArgumentConverters,
				  unsigned int Index0, unsigned int... Indices, typename PolicyList
				>
		int invoke3(lua_State* L, function_object const& self, invoke_context& ctx, F& f,
					 PolicyList, meta::index_list< Index0, Indices... > index_list, meta::type_list<ReturnType, Arguments...> signature_list,
					 ReturnConverter return_converter, ArgumentConverters... argument_converters )
		{
			typedef typename call_detail_new::compute_invoke_values< ArgumentConverters... > invoke_values;
			typedef meta::type_list<Arguments...> argument_list_type;
			typedef meta::index_list<Indices...> argument_index_list;

			int const arguments = lua_gettop(L);
			int score = -1;
			
			if (invoke_values::arity == arguments) {
				score = match_deferred(L, typename invoke_values::stack_index_list(), argument_list_type(), argument_converters...);
			}
			
			if (score >= 0 && score < ctx.best_score) {
				ctx.best_score = score;
				ctx.candidates[0] = &self;
				ctx.candidate_index = 1;
			}
			else if (score == ctx.best_score) {
				ctx.candidates[ctx.candidate_index++] = &self;
			}

			int results = 0;

			if (self.next)
			{
				results = self.next->call(L, ctx);
			}

			if (score == ctx.best_score && ctx.candidate_index == 1)
			{
				do_call_struct<F, std::is_void<ReturnType>::value>::do_call(L, f, typename invoke_values::stack_index_list(), argument_list_type(), return_converter, argument_converters...);
				meta::expand_calls_hack( (argument_converters.converter_postcall(L, decorated_type<Arguments>(), meta::get< typename invoke_values::stack_index_list, Indices-1 >::value), 0)... );
				
				results = lua_gettop(L) - invoke_values::arity;
				if (has_call_policy<PolicyList, yield_policy>::value) {
					results = lua_yield(L, results);
				}
				
				// call policiy list postcall
				call_detail_new::policy_list_postcall < PolicyList, typename meta::push_front< typename invoke_values::stack_index_list, meta::index<invoke_values::arity> >::type >::postcall( L, results );
			}

			return results;
		}

		template< typename F, typename ReturnType, typename... Arguments, unsigned int Index0, unsigned int... Indices, typename PolicyList >
		int invoke2(lua_State* L, function_object const& self, invoke_context& ctx, F& f,
					 meta::type_list<ReturnType, Arguments...> signature, meta::index_list<Index0,Indices...>, PolicyList)
		{
			typedef meta::type_list<ReturnType, Arguments...> signature_type;
			using return_converter = applied_converter_policy<0, PolicyList, ReturnType, cpp_to_lua>;
			return invoke3(L, self, ctx, f, 
				PolicyList(), meta::index_list<Index0,Indices...>(), signature,
				return_converter(), applied_converter_policy<Indices,PolicyList,Arguments,lua_to_cpp>()...
				);
		}

		template <class F, class Signature, typename... PolicyInjectors>
		// boost::bind's operator() is const, std::bind's is not
		inline int invoke(lua_State* L, function_object const& self, invoke_context& ctx, F& f, Signature,
			meta::type_list< PolicyInjectors... > const& injectors)
		{
			return invoke2(L, self, ctx, f, Signature(), typename meta::make_index_range<0, meta::size<Signature>::value>::type(), injectors);
		}

	}
} // namespace luabind::detail

# endif // LUABIND_CALL2_080911_HPP

