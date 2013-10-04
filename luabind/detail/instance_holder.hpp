// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_INSTANCE_HOLDER_081024_HPP
# define LUABIND_INSTANCE_HOLDER_081024_HPP

# include <luabind/detail/inheritance.hpp>
# include <luabind/get_pointer.hpp>
# include <luabind/typeid.hpp>
# include <stdexcept>

namespace luabind { 
	namespace detail {

		class instance_holder
		{
		public:
			instance_holder(bool pointee_const)
			  : m_pointee_const(pointee_const)
			{}

			virtual ~instance_holder()
			{}

			virtual std::pair<void*, int> get(cast_graph const& casts, class_id target) const = 0;
			virtual void release() = 0;

			bool pointee_const() const
			{
				return m_pointee_const;
			}

		private:
			bool m_pointee_const;
		};

		template <class P, class Pointee = void const>
		class pointer_holder : public instance_holder
		{
		public:
			pointer_holder( P p, class_id dynamic_id, void* dynamic_ptr ) :
				instance_holder( detail::is_pointer_to_const<P>() ),
				p(std::move(p)), weak(0), dynamic_id(dynamic_id), dynamic_ptr(dynamic_ptr)
			{
			}

			std::pair<void*, int> get(cast_graph const& casts, class_id target) const
			{
				// if somebody wants the smart-ptr, he can get a reference to it
				if (target == registered_class<P>::id) return std::pair<void*, int>(&this->p, 0);

				void* naked_ptr = const_cast<void*>(static_cast<void const*>(weak ? weak : get_pointer(p)));
				if (!naked_ptr) return std::pair<void*, int>(nullptr, 0);

				return casts.cast( naked_ptr,
								  registered_class< typename pointer_traits<P>::value_type >::id
								  , target, dynamic_id, dynamic_ptr );
			}

			explicit operator bool() const
			{
				return p ? true : false;
			}

			void release()
			{
				weak = const_cast<void*>(static_cast<void const*>(get_pointer(p)));
				release_ownership(p);
			}

		private:
			mutable P p;
			// weak will hold a possibly stale pointer to the object owned
			// by p once p has released it's owership. This is a workaround
			// to make adopt() work with virtual function wrapper classes.
			void* weak;
			class_id dynamic_id;
			void* dynamic_ptr;
		};

	}
} // namespace luabind::detail

#endif // LUABIND_INSTANCE_HOLDER_081024_HPP

