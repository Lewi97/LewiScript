#pragma once

#include "common.h"
#include "format_errs.h"

#include <span>

namespace le
{
	struct RuntimeValue
	{
		using LeObject = std::shared_ptr<RuntimeValue>;

		/*
		* An identifier for important builtin types. Custom types can only be interfaced by their virtual functions.
		*/
		enum class Type
		{
			Null, NumericLiteral, Variable, String, Function, Boolean, Module, Custom
		};

		Type type{};

		virtual auto copy() const -> LeObject
		{
			throw(ferr::make_exception("This object cannot be copied"));
			return LeObject{};
		}

		/* Declared as friend to place it within global namespace and also be able to use the function here */
		friend inline auto to_string(RuntimeValue::Type type) -> String
		{
			switch (type)
			{
			case RuntimeValue::Type::Null:
				return "Null";
			case RuntimeValue::Type::String:
				return "StringLiteral";
			case RuntimeValue::Type::NumericLiteral:
				return "NumericLiteral";
			case RuntimeValue::Type::Variable:
				return "Variable";
			case RuntimeValue::Type::Function:
				return "Function";
			case RuntimeValue::Type::Boolean:
				return "Boolean";
			case RuntimeValue::Type::Module:
				return "Module";
			case RuntimeValue::Type::Custom:
				return "Custom";
			default:
				return "Unknown";
			}
		}

		virtual auto make_string() -> String
		{
			return String("Null");
		}

		/*
		* @deprecated THIS OVERLOAD IS ONLY USED BY THE TREE INTERPRETER WHICH IS NO LONGER SUPPORTED
		* To clarify, the builtin function object does not use this overload, 
		it is to be used by objects that want an overloaded call operator.
		* The builtin function object requires the interpreter and storage.
		*/
		virtual auto call(std::span<LeObject>& args) -> LeObject
		{
			throw(ferr::make_exception("This object cannot be called"));
			return {};
		}

		/*
		* Overload used by bytecode compiled functions
		*/
		virtual auto call(std::span<LeObject>& args, struct VirtualMachine& vm) -> LeObject
		{
			throw(ferr::make_exception("This object cannot be called"));
			return {};
		}

		virtual auto apply_operation(Token::Type op, LeObject other) -> LeObject
		{
			throw(ferr::invalid_operation(op, to_string(type), to_string(other->type)));
			return {};
		}

		virtual auto apply_operation(Token::Type op) -> LeObject
		{
			throw(ferr::invalid_operation(op, to_string(type)));
			return LeObject{};
		}

		virtual auto to_native_bool() const -> bool
		{
			return false;
		}
		
		/*
		* expr[expr]
		*/
		virtual auto access(LeObject index) -> LeObject
		{
			throw(ferr::invalid_access(to_string(type), to_string(index->type)));
			return LeObject{};
		}

		/*
		* expr[expr] = expr
		*/
		virtual auto access_assign(LeObject index, LeObject new_val) -> LeObject
		{
			throw(ferr::invalid_access(to_string(type), to_string(index->type)));
			return LeObject{};
		}
	};

	using LeObject = RuntimeValue::LeObject;
	/*
	* Any imported function is expected to have the following interface.
	* The first param is a pointer to objects with the second being the count.
	* Do not use this interface outside of C++ to keep integrity of shared ptr's.
	*/
	using FFI_FUNC = LeObject(*)(std::span<LeObject>);

	struct NullValue : RuntimeValue
	{
		NullValue()
		{
			type = Type::Null;
		}

		auto make_string() -> String override
		{
			return String("Null");
		}
	};
}

