#ifndef _LL_CODE_GEN_
#define _LL_CODE_GEN_

#include "symbol_table.h"
#include <string>
#include <utility>
#include <list>
#include <map>
#include <vector>
#include <stdexcept>

namespace ToyLLGen
{
	struct InvalidOperation: public std::runtime_error
	{
		InvalidOperation(): std::runtime_error("Invalid Operation"){}
		InvalidOperation(const std::string what): std::runtime_error(what){}
	};

	struct UndefinedSymbol: public std::runtime_error
	{
		UndefinedSymbol(): std::runtime_error("Undefined Symbol"){}
		UndefinedSymbol(const std::string what): std::runtime_error(what){}
	};

	struct TypeError: public std::runtime_error
	{
		TypeError(): std::runtime_error("Type Error"){}
		TypeError(const std::string what): std::runtime_error(what){}
	};

	enum UnaryOperation
	{
		UO_MINUS,
		UO_NOT
	};

	enum BinaryOperation
	{
		BO_ADD,
		BO_SUB,
		BO_MUL,
		BO_DIV,
	
		BO_EQ,
		BO_NE,
		BO_GT,
		BO_GE,
		BO_LT,
		BO_LE
	};

	enum Type
	{
		T_VOID,
		T_CHAR,
		T_INT,
		T_FLOAT,
		T_DOUBLE
	};

	enum BlockType{
		BT_GLOBAL,
		BT_FUNC,
		BT_IF,
		BT_ELSE,
		BT_WHILE
	};


	class Block
	{
	public:	
		int m_tempCounter; //To make unique number

		Block();//GlobalBlock
		Block(const BlockType blockType, const Type returnType, const string &funcName, Block *parentBlock, const int tempCounter);
		//~Block();

		string endBlock();
		void setBlock(Scope *globalScope);
		void setBlock(const BlockType blockType, const Type returnType, const string &funcName, Scope *blockScope,Block *parentBlock,const int tempCounter);

		bool isGlobal() {return m_blockType == BT_GLOBAL ;}
		//For Functions
		void asFuncBlock(const list<pair<Type, string > > &params=list<pair<Type, string > >());
		void addVariableDecl(const Type type, const string &name, int arraySize = 0, bool is_arg = false , bool no_insert = false);
		string endFuncBlock();

		void print(const string &printTar);
		Block *getParentBlock(){ m_parentBlock->m_tempCounter = m_tempCounter;return m_parentBlock;}

		void asIfBlock(const string &cond);
		void asElseBlock();
		void endElseBlock();
		string endIfBlock();

		void beginWhileCondEvalution();//This will evaluate whether the while should go on
		void asWhileBlock(const string &cond);
		void endWhileBlock();

		std::string loadMemory(const std::string &name, const std::string &sub="");

		string addUnaryExpr(UnaryOperation, const string &rhs);
		string addBinaryExpr(const string &lhs, BinaryOperation, const string &rhs);
		string addAssignStmt(const string &lhs, const string &rhs);
		string addFunctionCall(const string &func, const list<string> &params);
		
		void catLL(string);
	private:
		string getSymbolType(const string &name);
		string allocTemp(const string &type);
		string getElementType(const string &type);
		Type stringToType(const string & in);

		int isTempVar(const string &name);
		bool isSymbol(const string &name) const;
		bool isPointerType(const string &type);

		string ref(const string &type);
		string genBasicOperation_i32(BinaryOperation, const string &lhs, const string &rhs, string &outType);
		string genBasicOperation_floating(BinaryOperation, const string &lhs, const string &rhs, string &outType);
		void prepare(const string &name, string &outNameValue,
			string &outType, bool dontLoad=false);//When doing operating it will load elemnt from symbol table, and assign to temp
		bool isArray(const string &type);
		
		
		string typeCasting(const string &type, const string &var, const string &targetType);

		Block *m_parentBlock;
		Scope *m_blockScope;
		BlockType m_blockType;
		Type m_RType;

		Symbol m_symbol;

		string m_funcName;

		map<string, string> m_addressedParameter;
		vector<Block *> m_childBlock; 
		map<string, string> m_recentData; // For optimize so it won't find the variable in memory
		//vector<bool> m_endBlock; // For dealing while, if else, itself's end sign
		
		string m_returnType;
		string returnVar;
		string m_ll;
		string m_funcLL;
		string *m_currentTargetLL;//
		string m_warningLog;

		string returnBranch_1;
		string returnBranch_2;


	};

}

#endif