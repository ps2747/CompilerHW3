#ifndef __LLGEN_SYMBOL_H_
#define __LLGEN_SYMBOL_H_


#include <string>
#include <list>

using namespace std;

namespace ToyLLGen
{
	struct Symbol
	{
		string type;
		string name;
		string uniqueName;
		int arraySize;
		list<string> paramTypes;

		Symbol(): arraySize(0){}

		bool isArray() const 
		{ return arraySize>0; }

		bool isFunction() const
		{ return paramTypes.size()>0; }
	};
}

#endif