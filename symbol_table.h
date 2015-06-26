#ifndef __SLLGEN_SYMBOL_TABLE_H_
#define __SLLGEN_SYMBOL_TABLE_H_

#include <string>
#include <map>
#include <list>
#include <stdexcept>
#include "symbol.h"

using namespace std;

namespace ToyLLGen
{
	class SymbolRedeclare: exception
	{
	public:
		SymbolRedeclare(): exception(){}
	};

	class Scope
	{
	public:
		Scope(Scope *parent);
		~Scope();

		//Insert will first check if the symbol name is existed
		void insert(const string &, const Symbol &); // when called, it will get counter ++, which for liu shuey hao
		bool lookUp(const string &, Symbol &) const;
		bool getUniqueName(const string &name, string &out, int level=0) const;

		void printSymbols(int level);

		Scope *addChild();
		Scope *getParent() const
		{ return m_parent; }

		map<string, Symbol> &getSymbols()
		{ return m_symbols; }
	private:
		Scope *m_parent;
		list<Scope *> m_children;
		map<string, Symbol> m_symbols;
		static int counter;
	};

	class SymbolTable
	{
	public:
		SymbolTable();
		~SymbolTable();

		void push();
		void pop();
		bool isInGlobal() const
		{ return m_globalScope==m_currentScope; }

		void insert(const string &, const Symbol &);
		bool lookUp(const string &, Symbol &) const;
		bool getUniqueName(const string &name, string &out) const;

		void setToGlobal(){m_currentScope = m_globalScope;}

		void setCurrentScope(Scope *scope){m_currentScope = scope;}

		Scope *getCurrentScope() const
		{ return m_currentScope; }

		void printSymbolTable();
		void printSymbolNode(int level);

		void clear();
	private:
		Scope *m_globalScope;
		Scope *m_currentScope;
	};
}

#endif