#include "symbol_table.h"

using namespace std;

namespace ToyLLGen
{
	int Scope::counter=0;
	Scope::Scope(Scope *parent):
		m_parent(parent)
	{

	}

	Scope::~Scope()
	{
		for(auto child: m_children)
			delete child;
	}

	void Scope::insert(const string &name, const Symbol &var)
	{
		Symbol symbol;
		symbol=var;
		symbol.uniqueName=(m_parent?"%": "@")+name;
		if(!(name[0]>='0'&&name[0]<='9'))
			symbol.uniqueName+=to_string(counter++);
		auto it=m_symbols.find(name);
		if(it==m_symbols.cend())
			m_symbols[name]=symbol;
		else
			throw SymbolRedeclare();
	}

	void Scope::printSymbols(int level){
		for(auto iterator = m_symbols.cbegin(); iterator != m_symbols.cend(); iterator++)
			printf(" %d  %s  %s %s %s\n", level, iterator->second.type.c_str(),iterator->second.name.c_str(),iterator->second.isArray() ? "Array" : "Not_Array",iterator->second.isFunction() ? "Func" :"Not_Func");
		++level;
		for(auto childIt = m_children.cbegin(); childIt != m_children.cend(); childIt++)
			(*childIt)->printSymbols(level);	
	}

	bool Scope::lookUp(const string &name, Symbol &out_var) const
	{
		auto it=m_symbols.find(name);
		if(it!=m_symbols.cend())
		{
			out_var=it->second;
			return true;
		}

		if(m_parent)
			return m_parent->lookUp(name, out_var);
		else
			return false;
	}

	bool Scope::getUniqueName(const string &name, string &out_name, int level) const
	{
		Symbol out_var;
		bool result=lookUp(name, out_var);
		out_name=out_var.uniqueName;
		return result ;
	}

	Scope *Scope::addChild()
	{
		Scope *new_scope = new Scope(this);
		m_children.push_back(new_scope);
		return new_scope;
	}

	SymbolTable::SymbolTable()
		: m_globalScope(new Scope(nullptr))
	{
		m_currentScope=m_globalScope;
	}

	SymbolTable::~SymbolTable()
	{
		delete m_globalScope;
	}

	void SymbolTable::push()
	{
		m_currentScope=m_currentScope->addChild();
	}

	void SymbolTable::pop()
	{
		if(m_currentScope!=m_globalScope)
			m_currentScope=m_currentScope->getParent();
	}

	void SymbolTable::insert(const string &name, const Symbol &var)
	{
		m_currentScope->insert(name, var);
	}

	bool SymbolTable::lookUp(const string &name, Symbol &var) const
	{
		return m_currentScope->lookUp(name, var);
	}

	void SymbolTable::clear()
	{
		delete m_globalScope;
		m_globalScope = new Scope(nullptr);
		m_currentScope = m_globalScope;
	}

	bool SymbolTable::getUniqueName(const string &name, string &out) const
	{
		out="";
		return m_currentScope->getUniqueName(name, out);
	}

	void SymbolTable::printSymbolTable()
	{
		m_globalScope->printSymbols(-1);
	}
}