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

#ifndef LUABIND_VALUE_WRAPPER_CONVERTER_HPP_INCLUDED
#define LUABIND_VALUE_WRAPPER_CONVERTER_HPP_INCLUDED

#include "conversion_base.hpp"
#include <luabind\value_wrapper.hpp>
#include <type_traits>

namespace luabind {

	namespace detail {

		template <class U>
		struct value_wrapper_converter
		{
			typedef value_wrapper_converter<U> type;
			typedef std::true_type is_native;

			enum { consumed_args = 1 };

			template<class T>
			T apply(lua_State* L, by_const_reference<T>, int index)
			{
				return T(from_stack(L, index));
			}

			template<class T>
			T apply(lua_State* L, by_value<T>, int index)
			{
				return apply(L, by_const_reference<T>(), index);
			}

			template<class T>
			static int match(lua_State* L, by_const_reference<T>, int index)
			{
				return value_wrapper_traits<T>::check(L, index)
					? (std::numeric_limits<int>::max)() / LUABIND_MAX_ARITY
					: -1;
			}

			template<class T>
			static int match(lua_State* L, by_value<T>, int index)
			{
				return match(L, by_const_reference<T>(), index);
			}

			void converter_postcall(...) {}

			template<class T>
			void apply(lua_State* interpreter, T const& value_wrapper)
			{
				value_wrapper_traits<T>::unwrap(interpreter, value_wrapper);
			}
		};

	}

}

#endif