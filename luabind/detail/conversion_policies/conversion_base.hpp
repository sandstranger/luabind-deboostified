#pragma once
#include <type_traits>
#include <luabind\lua_include.hpp>
#include <luabind\detail\decorate_type.hpp>
#include <luabind\detail\has_get_pointer.hpp>

namespace luabind {

	namespace detail
	{
		struct conversion_policy_base {};	
		struct converter_policy_has_postcall_tag {};
	}

	
	template< bool HasArg = true>
	struct conversion_policy : detail::conversion_policy_base
	{
		static const bool has_arg = HasArg;
	};


	class index_map
	{
	public:
		index_map(const int* m) : m_map(m) {}

		int operator [](int index) const
		{
			return m_map[index];
		}

	private:
		const int* m_map;
	};

	// TODO: What's this good for?
	struct indirection_layer
	{
		template<class T>
		indirection_layer(const T&);
	};

	namespace detail {

		// Something's strange with the references here... need to know when to copy :(
		template <class T, class Clone>
		void make_pointee_instance(lua_State* L, T& x, std::true_type, Clone)
		{
			if (get_pointer(x))
			{
				make_instance(L, std::move(x));
			}
			else
			{
				lua_pushnil(L);
			}
		}

		template <class T>
		void make_pointee_instance(lua_State* L, T& x, std::false_type, std::true_type)
		{
			typedef typename std::remove_reference<T>::type value_type;

			std::unique_ptr<value_type> ptr(new value_type(std::move(x)));
			make_instance(L, std::move(ptr));
		}

		template <class T>
		void make_pointee_instance(lua_State* L, T& x, std::false_type, std::false_type)
		{
			make_instance(L, &x);
		}

		template <class T, class Clone>
		void make_pointee_instance(lua_State* L, T& x, Clone)
		{
			make_pointee_instance(L, std::move(x), has_get_pointer<T>(), Clone());
		}
	}

	template <class T, class Enable = void>
	struct default_converter;

}