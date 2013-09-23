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

#ifndef LUABIND_VALUE_CONVERTER_HPP_INCLUDED
#define LUABIND_VALUE_CONVERTER_HPP_INCLUDED

#include <type_traits>
#include <luabind\lua_include.hpp>

namespace luabind {

	namespace detail {

		struct value_converter
		{
			typedef value_converter type;
			typedef std::false_type is_native;

			enum { consumed_args = 1 };

			value_converter()
				: result(0)
			{}

			void* result;

			template<class T>
			void apply(lua_State* L, T x)
			{
				if (luabind::get_back_reference(L, x))
					return;

				make_pointee_instance(L, std::move(x), std::true_type());
			}

			template<class T>
			T apply(lua_State*, by_value<T>, int)
			{
				return *static_cast<T*>(result);
			}

			template<class T>
			int match(lua_State* L, by_value<T>, int index)
			{
				// special case if we get nil in, try to match the holder type
				if (lua_isnil(L, index))
					return -1;

				object_rep* obj = get_instance(L, index);
				if (obj == 0) return -1;

				std::pair<void*, int> s = obj->get_instance(registered_class<T>::id);
				result = s.first;
				return s.second;
			}

			template<class T>
			void converter_postcall(lua_State*, T, int) {}
		};

	}

}

#endif