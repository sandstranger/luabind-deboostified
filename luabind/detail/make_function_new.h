#pragma once
#include "meta.h"
#include "signature_new.h"
#include "object.hpp"

namespace exp {

	/*
		Convert table to container
		Convert container to table ...

		Support set, vector and map

	
	// Lua -> cpp
	template< typename T >
	struct argument_converter {
		
		
		static unsigned int numpop = 1;
	
		T get( lua_State* l );
	
	};


	
	
	*/






	class function_object_base {
		virtual std::string format_signature() = 0;
		virtual ~function_object_base() {}

		virtual int call_from_lua(lua_State* L);

		static int entry_point(lua_State* L);
	};

	template< typename CallableType >
	class function_object :
		public function_object_base
	{
	public:
		typedef deduce_signature< CallableType > signature_type;

	public:
		function_object(lua_State* luaState, const CallableType& callable)
			: callable_(callable)
		{
		}

		function_object(lua_State* luaState, CallableType && callable)
			: callable_(std::move(callable))
		{
		}

		void call_lua(lua_State* L)
		{
		}

	private:
		CallableType callable_;
	};
	
	template< typename CallableType >
	std::unique_ptr<function_object_base> make_function_base_object(lua_State* luaState, CallableType&& callable)
	{
		return std::make_unique < function_object<typename std::remove_reference<CallableType>::type>(luaState, std::forward<CallableType>(callable));
	}

}