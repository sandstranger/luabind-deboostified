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


#ifndef LUABIND_DISCARD_RESULT_POLICY_HPP_INCLUDED
#define LUABIND_DISCARD_RESULT_POLICY_HPP_INCLUDED

#include <luabind/config.hpp>
#include <luabind/detail/policy.hpp>    // for index_map, etc
#include <luabind/detail/primitives.hpp>  // for null_type, etc
#include <luabind/lua_include.hpp>

namespace luabind { namespace detail 
{
	struct discard_converter
	{
		template<class T>
		void apply(lua_State*, T) {}
	};

	struct discard_result_policy : conversion_policy<>
	{
		struct can_only_convert_from_cpp_to_lua {};

		template<class T, class Direction>
		struct apply
		{
			static_assert( std::is_same< Direction, cpp_to_lua >::value, "Can only convert from cpp to lua" );
			typedef discard_converter type;
		};
	};

}}

namespace luabind
{
  typedef meta::type_list< converter_policy_injector< 0, detail::discard_result_policy > > discard_result_list;

  inline discard_result_list discard_result()
  {
	  return discard_result_list();
  }
}

#endif // LUABIND_DISCARD_RESULT_POLICY_HPP_INCLUDED

