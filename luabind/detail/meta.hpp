// Copyright Michael Steinberg 2013. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <tuple>

namespace meta {

	struct type_list_tag {};
	struct index_list_tag {};

	/*
	Index list and type list share pretty common patterns... is there a way to unify them?
	*/

	template< unsigned int >
	struct count
	{};

	template< unsigned int >
	struct index
	{};

	// Use this to unpack a parameter pack into a list if T's
	template< typename T, typename DontCare >
	struct unpack_helper
	{
		typedef T type;
	};

	// common operators
	template< typename T >
	struct size;

	template< typename T, unsigned int Index >	// Specializations so index lists can use the same syntax
	struct get;

	template< typename... Lists >
	struct join;

	template< typename List1, typename List2, typename... Lists >
	struct join< List1, List2, Lists... > {
		// Could try to join on both sides
		typedef typename join< typename join< List1, List2 >::type, Lists... >::type type;
	};

	// convenience
	template< typename T >
	struct front : public get< T, 0 >
	{
	};

	template< typename List, typename T >
	struct push_front;

	template< typename List, typename T >
	struct push_back;

	template< typename T >
	struct pop_front;

	template< typename T >
	struct pop_back;

	template< typename List, unsigned int Index, typename T >
	struct replace;

	template< typename List, unsigned int Index, template< typename > class T >
	struct enwrap;

	template< typename List, template< typename > class T >
	struct enwrap_all;

	template< typename List, unsigned int Index, template< typename > class T >
	struct transform;

	template< typename List, template< typename > class T >
	struct transform_all;

	template< typename T, unsigned int start, unsigned int end >
	struct sub_range;

	// Used as terminator on type and index lists
	struct null_type {};

	template< typename... Types >
	struct type_list : public type_list_tag
	{
	};

	template< typename T >
	struct is_typelist : public std::false_type
	{
		static const bool value = false;
	};

	template< typename... Types >
	struct is_typelist< type_list< Types... > > : public std::true_type
	{
		static const bool value = true;
	};

	/*
	Find type
	*/

	template< typename TypeList, typename Type >
	struct contains;

	template< typename Type0, typename... Types, typename Type >
	struct contains< type_list<Type0, Types...>, Type >
		: std::conditional< std::is_same<Type0, Type>::value, std::true_type, contains< type_list<Types...>, Type > >::type
	{
	};

	template< typename Type >
	struct contains< type_list< >, Type >
		: std::false_type
	{
	};

	/*
	size
	*/

	template< >
	struct size< type_list< > >  {
		enum { value = 0 };
	};

	template< typename Type0, typename... Types >
	struct size< type_list< Type0, Types... > >  {
		enum { value = 1 + size< type_list<Types...> >::value };
	};


	template< typename... Types, typename Type >
	struct push_front< type_list<Types...>, Type >
	{
		typedef type_list< Type, Types... > type;
	};

	template< typename... Types, typename Type >
	struct push_back< type_list<Types...>, Type >
	{
		typedef type_list< Types..., Type > type;
	};

	/*
	pop_front
	*/

	template< typename Type0, typename... Types >
	struct pop_front< type_list< Type0, Types... > >
	{
		typedef typename type_list< Types... > type;
	};

	template< >
	struct pop_front< type_list< > >
	{
		typedef type_list< > type;
	};

	/*
	pop_back
	*/
	template< typename TypeN, typename... Types >
	struct pop_back< type_list< Types..., TypeN > >
	{
		typedef typename type_list< Types... > type;
	};

	template< >
	struct pop_back< type_list< > >
	{
		typedef type_list< > type;
	};

	/*
	Index access to type list
	*/

	template< typename Element0, typename... Elements, unsigned int Index >
	struct get< type_list<Element0, Elements...>, Index > {
		typedef typename get< typename type_list<Elements...>, Index - 1 >::type type;
	};

	template< typename Element0, typename... Elements >
	struct get< type_list<Element0, Elements...>, 0 >
	{
		typedef Element0 type;
	};

	template< unsigned int Index >
	struct get< type_list< >, Index >
	{
		static_assert(size< type_list< int > >::value == 1, "Bad Index");
	};

	template< typename... Types1, typename... Types2 >
	struct join< type_list< Types1... >, type_list< Types2... > >
	{
		typedef type_list< Types1..., Types2... > type;
	};

	/*
	template< typename List, unsigned int Index, typename T >
	struct replace;
	*/

	namespace detail {
		template< typename HeadList, typename TailList, typename Type, unsigned int Index >
		struct replace_helper;

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, typename Type, unsigned int Index >
		struct replace_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, Index> {
			typedef typename replace_helper< type_list< HeadTypes..., CurrentType >, type_list<TailTypes...>, Type, Index - 1 >::type type;
		};

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, typename Type >
		struct replace_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, 0 > {
			typedef type_list< HeadTypes..., Type, TailTypes... > type;
		};
	}

	template< typename... Types, unsigned int Index, typename Type >
	struct replace< type_list< Types... >, Index, Type >
	{
		typedef type_list< Types... > TypeList;

		typedef typename meta::join< typename meta::sub_range< TypeList, 0, Index >::type, meta::type_list<Type>, typename meta::sub_range < TypeList, Index + 1, sizeof...(Types) >::type >::type type;
		
		//typedef typename detail::replace_helper< type_list<>, type_list<Types...>, Type, Index >::type type;
	};

	namespace detail {
		template< typename HeadList, typename TailList, template< typename > class Type, unsigned int Index >
		struct enwrap_helper;

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, template< typename > class Type, unsigned int Index >
		struct enwrap_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, Index> {
			typedef typename enwrap_helper< type_list< HeadTypes..., CurrentType >, type_list<TailTypes...>, Type, Index - 1 >::type type;
		};

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, template< typename > class Type >
		struct enwrap_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, 0> {
			typedef type_list< HeadTypes..., Type<CurrentType>, TailTypes... > type;
		};
	}

	template< typename... Types, unsigned int Index, template< typename >  class Type >
	struct enwrap< type_list< Types... >, Index, Type > {
		typedef typename detail::enwrap_helper< type_list< >, type_list< Types... >, Type, Index >::type type;
	};

	template< typename... Types, template< typename > class Enwrapper >
	struct enwrap_all< type_list< Types... >, Enwrapper >
	{
		typedef type_list< Enwrapper< Types >... > type;
	};

	namespace detail {
		template< typename HeadList, typename TailList, template< typename > class Type, unsigned int Index >
		struct transform_helper;

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, template< typename > class Type, unsigned int Index >
		struct transform_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, Index> {
			typedef typename transform_helper< type_list< HeadTypes..., CurrentType >, type_list<TailTypes...>, Type, Index - 1 >::type type;
		};

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, template< typename > class Type >
		struct transform_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, 0> {
			typedef type_list< HeadTypes..., typename Type<CurrentType>::type, TailTypes... > type;
		};
	}

	template< typename... Types, unsigned int Index, template< typename >  class Type >
	struct transform< type_list< Types... >, Index, Type > {
		typedef typename detail::transform_helper< type_list< >, type_list< Types... >, Type, Index >::type type;
	};


	template< typename Type0, typename... Types, template< typename >  class Type >
	struct transform_all< type_list< Type0, Types... >, Type > {
		typedef typename push_front< typename transform_all< type_list<Types...>, Type >::type, typename Type<Type0>::type >::type type;
	};

	template< template< typename >  class Type >
	struct transform_all< type_list< >, Type > {
		typedef type_list< > type;
	};


	/*
	Tuple from type list
	*/
	template< class TypeList >
	struct make_tuple;

	template< typename... Types >
	struct make_tuple< type_list< Types... > >
	{
		typedef std::tuple< Types... > type;
	};


	/*
	Type selection
	*/

	template< typename ConvertibleToTrueFalse, typename Result >
	struct case_ : public ConvertibleToTrueFalse {
		typedef Result type;
	};

	template< typename Result >
	struct default_ {
		typedef Result type;
	};


	template< typename Case, typename... CaseList >
	struct select_
	{
		typedef typename std::conditional< std::is_convertible<Case, std::true_type>::value, typename Case::type, typename select_<CaseList...>::type >::type type;
	};

	template< typename Case >
	struct select_< Case >
	{
		typedef typename std::conditional< std::is_convertible<Case, std::true_type>::value, typename Case::type, null_type >::type type;
	};

	template< typename T >
	struct select_< default_<T> > {
		typedef typename default_<T>::type type;
	};

	/*
	Create index lists to expand on type_lists
	*/

	template< unsigned int... Indices >
	struct index_list : public index_list_tag
	{
	};

	/*
	Index index list
	*/

	template< unsigned int Index0, unsigned int... Indices, unsigned int Index >
	struct get< index_list<Index0, Indices...>, Index > {
		enum { value = typename get< index_list<Indices...>, Index - 1 >::value };
	};

	template< unsigned int Index0, unsigned int... Indices >
	struct get< index_list<Index0, Indices...>, 0 >
	{
		enum { value = Index0 };
	};

	template< unsigned int Index >
	struct get< index_list< >, Index >
	{
		static_assert(size< index_list< Index > >::value == 1, "Bad Index");
	};

	template< >
	struct get< index_list< >, 0 >
	{
	};

	/*
	Index list size
	*/

	template< >
	struct size< index_list< > >  {
		enum { value = 0 };
	};

	template< unsigned int Index0, unsigned int... Indices >
	struct size< index_list< Index0, Indices... > >  {
		enum { value = 1 + size< index_list< Indices... > >::value };
	};

	template< unsigned int... Indices, unsigned int Index >
	struct push_front< index_list< Indices... >, index<Index> >
	{
		typedef index_list< Index, Indices... > type;
	};

	template< unsigned int... Indices, unsigned int Index >
	struct push_back< index_list< Indices... >, index<Index> >
	{
		typedef index_list< Indices..., Index > type;
	};

	/*
	pop_front
	*/

	template< unsigned int Index0, unsigned int... Indices >
	struct pop_front< index_list< Index0, Indices... > > {
		typedef index_list< Indices... > type;
	};

	template< >
	struct pop_front< index_list< > > {
		typedef index_list<  > type;
	};


	namespace detail {

		template< unsigned int curr, unsigned int end, unsigned int... Indices >
		struct make_index_range :
			public make_index_range< curr + 1, end, Indices..., curr >
		{
		};

		template< unsigned int end, unsigned int... Indices >
		struct make_index_range< end, end, Indices... >
		{
			typedef index_list< Indices... > type;
		};

	}

	template< unsigned int start, unsigned int end >
	struct make_index_range {
		typedef typename detail::make_index_range< start, end >::type type;
	};

	/*
	Exracts the first N elements of an index list and creates a new index list from them
	*/

	namespace detail {

		template< typename SourceList, typename IndexList >
		struct sub_range_index;

		template< typename SourceList, unsigned int... Indices >
		struct sub_range_index< SourceList, index_list< Indices... > > {
			typedef index_list< get< SourceList, Indices >::value... > type;
		};

		template< typename SourceList, typename IndexList >
		struct sub_range_type;

		template< typename SourceList, unsigned int... Indices >
		struct sub_range_type< SourceList, index_list< Indices... > > {
			typedef type_list< typename get< SourceList, Indices >::type... > type;
		};

	}

	template< unsigned int start, unsigned int end, unsigned int... Indices >
	struct sub_range< index_list<Indices...>, start, end >
	{
		typedef typename detail::sub_range_index< index_list<Indices...>, typename make_index_range<start, end>::type >::type type;
	};

	template< unsigned int start, unsigned int end, typename... Types >
	struct sub_range< type_list<Types...>, start, end >
	{
		typedef typename detail::sub_range_type< type_list<Types...>, typename make_index_range<start, end>::type >::type type;
	};

	template< typename IndexList, unsigned int Index >
	struct push_back_index;

	template< unsigned int... Indices, unsigned int Index >
	struct push_back_index< index_list< Indices... >, Index >
	{
		typedef index_list< Indices..., Index > type;
	};


	template< typename T >
	struct sum;
	
	template< unsigned int Arg0, unsigned int... Args >
	struct sum< index_list<Arg0, Args...> >
	{
		enum{ value = Arg0 + sum<index_list<Args...>>::value };
	};

	template< >
	struct sum< index_list< > > {
		enum {value = 0};
	};

	/*
		and_ or_
	*/

	template< typename... ConvertiblesToTrueFalse >
	struct and_;

	template< typename Convertible0, typename... Convertibles >
	struct and_< Convertible0, Convertibles... >
		: std::conditional <
		std::is_convertible< Convertible0, std::true_type >::value,
		and_< Convertibles... >,
		std::false_type > ::type
	{
	};

	template< >
	struct and_< >
		: std::true_type
	{
	};


	template< typename... ConvertiblesToTrueFalse >
	struct or_;

	template< typename Convertible0, typename... Convertibles >
	struct or_< Convertible0, Convertibles... >
		: std::conditional <
		std::is_convertible< Convertible0, std::true_type >::value,
		std::true_type,
		or_< Convertibles... > 
		> ::type
	{
	};

	template< >
	struct or_< >
		: std::true_type
	{
	};


}