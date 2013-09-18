#pragma once
#include "meta.h"
#include <type_traits>
#include <functional>

namespace exp {

	/*
	struct signature {
	typedef type return_type;

	static const bool is_member;
	typedef type class_type;

	typedef type_list< arg types > argument_types;
	};
	*/

	namespace detail {
		struct empty_type {};
	}

	/*
		Helpers for function pointers, if needed
	*/

	template< typename Callable >
	struct decomposed_callable;

	template< typename R, typename... Args >
	struct decomposed_callable< R(*)(Args...) > {
		typedef R return_type;
		typedef detail::empty_type class_type;
		static const bool is_member = false;
		static const bool is_void = std::is_void<R>::value;
		typedef meta::type_list< Args... > argument_types;

	};

	template < typename Class, typename R, typename... Args >
	struct decomposed_callable < R(Class::*)(Args...) > {
		typedef R return_type;
		typedef Class class_type;
		typedef const bool is_member = false;
		typedef const bool is_void = std::is_void<R>::value;
		typedef meta::type_list< Args... > argument_types;
	};


	/*
		Irgendwie muss hier noch das Wrapped behandelt werden, wenn ich mal herausgefunden habe, wozu das gut ist
	*/

	template< typename DecomposedCallable, bool IsMember = DecomposedCallable::is_member, bool IsVoid = DecomposedCallable::is_void, typename ArgList = DecomposedCallable::argument_types >
	struct DecomposedCallableToPointerType;

	template< typename DecomposedCallable, bool IsVoid, typename... Args >
	struct DecomposedCallableToPointerType< DecomposedCallable, false, IsVoid, meta::type_list<Args...> >
	{
		typedef DecomposedCallable::Ret(*type)(Args...);
	};

	template< typename DecomposedCallable, bool IsVoid, typename... Args >
	struct DecomposedCallableToPointerType< DecomposedCallable, true, IsVoid, meta::type_list<Args...> >
	{
		typedef (typename DecomposedCallable::Ret)((typename DecomposedCallable::class_type)::*type)(Args...);
	};

	/*
	
	
	*/

	template< typename Ret, typename... Args >
	struct signature {
		typedef Ret return_type;
		typedef meta::type_list< Args... > argument_types;
		typedef Ret signature_type(Args...);
	};
	
	template< typename Callable >
	struct deduce_signature :
		public deduce_signature< decltype(&Callable::operator()) >
	{
	};

	template< typename Ret, typename... Args >
	struct deduce_signature < Ret(*)(Args...) > 
		: public signature<Ret, Args...> {};

	template< typename Class, typename Ret, typename... Args >
	struct deduce_signature < Ret(Class::*)(Args...) >
		: public signature<Ret, Args...> {};

}