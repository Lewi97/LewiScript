#pragma once

#include "common.h"

namespace le
{
	enum class OpCode
	{
		Halt,
		Pop,

		/* Builtin type creation */
		PushInt, /* UNUSED CURRENTLY */
		PushReal, /* For pushing number literals */
		
		PushGlobal, /* UNUSED */
		PushString, /* Pushes a string from the globally loaded strings, operand is index */
		PushFunction, /* Operand denotes index of Frame in functions vector */

		MakeArray, /* Operand denotes amount of values */

		/* Store */
		Store, /* Will store TOS at index of operand */

		/* Load */
		Load, /* Operand is index of variable */

		/* Subscripts */
		Access, /* Implements 'expr[expr]' Will attempt to access TOS with second to TOS */
		AccessAssign, /* Implements 'expr[expr] = expr' Target TOS, Query second to TOS, RHS third to TOS */

		/* Calling */
		Call, /* Implements 'expr()' , operand is number of args on stack */
		CallFunction, /* [DEPRECATED] Implements calling a builtin function, operand is number of args on stack */

		/* Controlflow */
		ReturnExpr, /* Implements 'return expr' Exits current function and pushes result of expr to stack of old scope */
		Return, /* Implements 'return' Exits current function */

		/* Jumps */
		Jump, /* Unconditional jump, operand stores delta */
		JumpIfTrue, /* Operand stores delta, can be negative or positive */
		JumpIfFalse, /* Operand stores delta, can be negative or positive */

		/* Operations */
		Add,
		Mul,
		Div,
		Sub,

		/* Relational */
		GT,
		GET,
		LT,
		LET,
		EQ,
		NEQ,
	};

#define LE_TO_STR(code) case OpCode::##code: return #code
	inline auto to_string(OpCode op) -> StringView
	{
		switch (op)
		{
			LE_TO_STR(Halt);	LE_TO_STR(Pop);
			LE_TO_STR(PushReal);LE_TO_STR(PushString); 
			LE_TO_STR(Load);	LE_TO_STR(Add);
			LE_TO_STR(Mul);		LE_TO_STR(Div);
			LE_TO_STR(Sub);		LE_TO_STR(Store);
			LE_TO_STR(MakeArray); LE_TO_STR(Access);
			LE_TO_STR(AccessAssign); LE_TO_STR(JumpIfTrue);
			LE_TO_STR(Jump); LE_TO_STR(GT);
			LE_TO_STR(GET); LE_TO_STR(LT);
			LE_TO_STR(LET); LE_TO_STR(EQ);
			LE_TO_STR(NEQ); LE_TO_STR(JumpIfFalse);
			LE_TO_STR(PushFunction); LE_TO_STR(Call);
			LE_TO_STR(CallFunction);
			LE_TO_STR(Return);
			LE_TO_STR(ReturnExpr);
		}
		return "Unknown opcode";
	}
#undef LE_TO_STR

	inline auto to_token_type(OpCode op) -> Token::Type
	{
		switch (op)
		{
		case OpCode::Add: return Token::Type::OperatorPlus;
		case OpCode::Mul: return Token::Type::OperatorMultiply;
		case OpCode::Div: return Token::Type::OperatorDivide;
		case OpCode::Sub: return Token::Type::OperatorMinus;
		case OpCode::GT: return Token::Type::OperatorGT;
		case OpCode::GET: return Token::Type::OperatorGET;
		case OpCode::LT: return Token::Type::OperatorLT;
		case OpCode::LET: return Token::Type::OperatorLET;
		case OpCode::EQ: return Token::Type::OperatorEq;
		case OpCode::NEQ: return Token::Type::OperatorNEq;
		default:
			throw(ferr::make_exception("Cannot convert opcode to token type"));
		}
	}

	struct Instruction
	{
		explicit Instruction(OpCode op_) : op(op_) {}
		Instruction(OpCode op_, u64 uinteger) 
			: op(op_)
		{
			operand.uinteger = uinteger;
		};
		Instruction(OpCode op_, i64 integer) 
			: op(op_)
		{
			operand.integer = integer;
		};
		Instruction(OpCode op_, double real) 
			: op(op_)
		{
			operand.real = real;
		};

		OpCode op{};
		/* OpCode will identify what the underlying value is */
		union {
			u64 uinteger;
			i64 integer;
			double real;
		} operand;
	};
	//constexpr auto instruction__size = sizeof(Instruction);

	/* Sequence of instructions */
	using ByteCode = std::vector<Instruction>;

	/* 
	* Function code frame
	*/
	struct Frame
	{
		ByteCode code{};
		String name{};
		int argc{};
	};

	/*
	* make a load global type to load from global space,
	*/
	struct Code
	{
		/* Global string storage */
		using Strings = std::vector<String>;
		using Functions = std::vector<Frame>;
		
		Functions functions{};
		ByteCode code{};
		Strings global_strings{};
	};

	constexpr auto frame__size = sizeof(Code);

	inline auto to_string(const Instruction& i, const Code& code, i64 count = 0) -> String
	{
		auto string = String();
		switch (i.op)
		{
			/* Op codes loading an index */
		case OpCode::Load: case OpCode::Store: case OpCode::MakeArray: case OpCode::Call: case OpCode::CallFunction:
			string += std::to_string(i.operand.uinteger); break;
			/* Jumps */
		case OpCode::Jump: case OpCode::JumpIfTrue: case OpCode::JumpIfFalse:
			string += std::format("{} -> {}", i.operand.integer, count + i.operand.integer); break;
			/* Push Builtin types */
		case OpCode::PushReal: string += std::to_string(i.operand.real); break;
		case OpCode::PushString:
			string += std::format("{} ({})", i.operand.uinteger, code.global_strings.at(i.operand.uinteger)); break;
		case OpCode::PushFunction:
			string += std::format("{} ({})", i.operand.uinteger, code.functions.at(i.operand.uinteger).name); break;
		}
		return string;
	}

	inline auto to_string(const ByteCode& code, const Code& context) -> String
	{
		auto string = String();

		for (auto count{ 0 }; const auto & i : code)
		{
			string += std::format("{}\t{} {}\n", count, to_string(i.op), to_string(i, context, count));
			count++;
		}

		return string;
	}

	inline auto to_string(const Frame& frame, const Code& context) -> String
	{
		return std::format("{}\n", to_string(frame.code, context));
	}

	inline auto to_string(const Code& code) -> String
	{
		auto string = to_string(code.code, code);
		for (const auto& function : code.functions)
			string += std::format("\n(Function '{}')\n{}\n", function.name, to_string(function, code));
		return string;
	}
}

