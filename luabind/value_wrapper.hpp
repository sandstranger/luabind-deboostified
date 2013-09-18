// Copyright (c) 2005 Daniel Wallin and Arvid Norberg

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

#ifndef LUABIND_VALUE_WRAPPER_050419_HPP
#define LUABIND_VALUE_WRAPPER_050419_HPP

#ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
# define LUABIND_USE_VALUE_WRAPPER_TAG 
#else
#endif

#include <type_traits>

namespace luabind {

//
// Concept ``ValueWrapper``
//

// TODO: Remove boost from conditional code
#ifdef LUABIND_USE_VALUE_WRAPPER_TAG
template<class T>
struct value_wrapper_traits;
    
namespace detail 
{ 

  BOOST_MPL_HAS_XXX_TRAIT_DEF(value_wrapper_tag);

  struct unspecialized_value_wrapper_traits
  {
      typedef std::false_type is_specialized;
  };

  template<class T>
  struct value_wrapper_traits_aux
  {
      typedef value_wrapper_traits<typename T::value_wrapper_tag> type;
  };

} // namespace detail
#endif

template<class T>
struct value_wrapper_traits
#ifdef LUABIND_USE_VALUE_WRAPPER_TAG
  : boost::mpl::eval_if<
        boost::mpl::and_<
            boost::mpl::not_<
                boost::mpl::or_<
                    boost::is_reference<T>
                  , boost::is_pointer<T>
                  , boost::is_array<T>
                >
            >
          , detail::has_value_wrapper_tag<T>
        >
      , detail::value_wrapper_traits_aux<T>
      , boost::mpl::identity<detail::unspecialized_value_wrapper_traits>
    >::type
{};
#else
{
    typedef std::false_type is_specialized;
};
#endif

template<class T>
struct is_value_wrapper
  : value_wrapper_traits<T>::is_specialized
{};

} // namespace luabind

namespace luabind {

template<class T>
struct is_value_wrapper_arg
  : is_value_wrapper<
      typename std::remove_const<
          typename std::remove_reference<T>::type
      >::type
    >
{};

template< class T >
struct is_value_wrapper_arg2
	: std::conditional < is_value_wrapper_arg<T>::value, std::true_type, std::false_type >::type
{};

} // namespace luabind

#endif // LUABIND_VALUE_WRAPPER_050419_HPP

