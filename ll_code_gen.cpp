#include "ll_code_gen.h"
#include "symbol_table.h"
#include "symbol.h"
#include <cstdlib>
#include <iostream>

using namespace std;

static const char *typeLLVMMap[]={"", "i8", "i32", "float", "double"};
// for variable symbol
static const char *typeLLVMMapPtr[]={"", "i8*", "i32*", "float*", "double*"};

static string ReplaceString(string subject, const string& search,
                          const string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

static ToyLLGen::SymbolTable symbolTable;

void printST()
{
	symbolTable.printSymbolTable();
}

namespace ToyLLGen{
	Block::Block() //Global block
	{
		Scope *globalScope = symbolTable.getCurrentScope();
		setBlock(globalScope);
	}
	Block::Block(const BlockType blockType, const Type returnType, const string &funcName, Block *parentBlock, const int tempCounter)
	{	
		symbolTable.push();
		Scope *blockScope = symbolTable.getCurrentScope();
		setBlock( blockType, returnType, funcName, blockScope, parentBlock, tempCounter);
	}
	//~Block();

	string Block::endBlock()
	{
		symbolTable.pop();
		return m_ll;
	}
 	void Block::setBlock(Scope *globalScope)
 	{
 		m_blockScope = globalScope;
 		m_parentBlock = NULL;
 		m_blockType = BT_GLOBAL;
 		m_tempCounter = 0;
 		m_RType = T_VOID;
 		m_symbol.paramTypes.clear();
 		m_funcName = "Global";
 	}
	void Block::setBlock(const BlockType blockType, const Type returnType, const string &funcName, Scope *blockScope,Block *parentBlock,const int tempCounter)
	{
		m_blockType = blockType;
		m_RType = returnType;
		m_funcName = funcName; 
		m_blockScope = blockScope;
		m_parentBlock = parentBlock;
		m_tempCounter = tempCounter;
		m_symbol.paramTypes.clear();

	}
	void Block::addVariableDecl(const Type type, const string &name, int arraySize, bool is_arg, bool no_insert)
	{
		// Make our symbol first
		Symbol var;
		var.type=string(typeLLVMMap[type])+(!is_arg?"*":"");
		var.name=name;
		var.arraySize=arraySize;

		string t_type=typeLLVMMap[type];
		string align="4"; // Char
		if(t_type=="double")
			align="8";
		if(var.isArray())
		{
			var.type=(t_type="["+to_string(var.arraySize)+" x "+t_type+"]")+"*";
			align="16";
		}

		symbolTable.insert(name, var);

		string uniqueName;
		symbolTable.getUniqueName(name, uniqueName);



		if(symbolTable.isInGlobal())
		{
			string initializer=(var.isArray()? "zeroinitializer": "0");
			m_ll+=uniqueName+" = global "+t_type+" "+initializer+", align "+align+"\n";
		}else
			m_ll+=uniqueName+" = alloca "+t_type+", align "+align+"\n";
	}

	//For Functions
	void Block::asFuncBlock(const list<pair<Type, string > > &params)
	{
		if(m_parentBlock -> isGlobal()){
			m_symbol.type = typeLLVMMap[m_RType];
			m_symbol.name = m_funcName;

			string args;

			//Push to paramType 
			m_symbol.paramTypes.push_back("FUNC");
			for (auto &p : params)
				m_symbol.paramTypes.push_back(typeLLVMMap[p.first]);
			symbolTable.insert(m_funcName, m_symbol);
			symbolTable.push();

			m_blockScope = symbolTable.getCurrentScope();
			for(auto &p : params){
				addVariableDecl(p.first, p.second, 0, true);
				string uniqueName ;
				symbolTable.getUniqueName(p.second, uniqueName);
				args = args+ typeLLVMMap[p.first] + " " + uniqueName +", ";
			}

			args = args.size()>0? args.substr(0, args.size()-2): args;//To take out ", "
			m_ll = "define "+ m_symbol.type+ " @"+ m_funcName +"("+args+")  {\n";

			symbolTable.push();

			if(m_symbol.type != "void"){
				auto rntVar=allocTemp(typeLLVMMapPtr[m_RType]);
				addVariableDecl(m_RType, rntVar.substr(1), 0, false, true);
			}

			for(auto &p: params)
			{
				auto tempVar=allocTemp(typeLLVMMapPtr[p.first]);

				string uniqueName;
				symbolTable.getUniqueName(p.second, uniqueName);
				m_addressedParameter[uniqueName]=tempVar;
				m_recentData[uniqueName]=uniqueName;

				addVariableDecl(p.first, tempVar.substr(1), 0, false, true);
				m_ll = m_ll + "store " + typeLLVMMap[p.first] + " "+uniqueName+", "+
					typeLLVMMapPtr[p.first]+" "+tempVar+"\n";
			}
		}else
			throw InvalidOperation();
	}

	string Block::endFuncBlock()
	{

		/*if(symbolTable.isInGlobal())
			throw InvalidOperation("InvalidOperation: endfunction must be placed in function");*/
		string varName, varType;
		prepare("%1", varName, varType);
		string exitLabel;
		auto tempVar = allocTemp(ref(varType));
		m_ll += tempVar+" = load "+varType+" %1, align 4\n";
		m_ll += "ret "+ref(varType)+" "+tempVar+"\n";
		m_ll += "}\n";
		symbolTable.pop();
		symbolTable.pop();

		m_ll+=ReplaceString(m_funcLL, "{EXIT_HERE}", exitLabel);

		return m_ll;
	}

	void Block::asIfBlock(const string &conditions)
	{
		if(symbolTable.isInGlobal())
			throw InvalidOperation("IfClause must be placed in function");
		string type;

		returnBranch_2 = conditions;

		string tmpVar=allocTemp("i1");
		m_ll+=tmpVar+" = icmp "+conditions +"\n";
		returnBranch_1=tmpVar;
		string trueLabel = allocTemp("label");
		m_ll += "br i1 "+conditions+", label "+trueLabel+", label {END_IF_TEMP}\n";

	}

	string Block::endIfBlock()
	{
		if(symbolTable.isInGlobal())
			throw InvalidOperation("InvalidOperation: endIf must be placed in function");
		size_t ifBrEnd = m_ll.find("{END_IF_TEMP}");
		string tempBrEnd = allocTemp("label");
		m_ll.replace(ifBrEnd, 13,tempBrEnd);
		m_ll += "br label %"+tempBrEnd+"\n";
		return m_ll;
	}
	/*void Block::asElseBlock()
	{

	}
	void Block::endElseBlock(){}
	*/

	void Block::catLL(string inLL)
	{
		m_ll+=inLL;
	}
	void Block::print(const string &printTar)
	{
		string outName, outType;
		prepare(printTar, outName, outType);
		m_ll += " call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), "+outType+" "+outName+")";
	}
	/*
	void Block::beginWhileCondEvalution(){}//This will evaluate whether the while should go on
	void Block::asWhileBlock(const string &cond){}
	void Block::endWhileBlock(){}
	*/
	string Block::addUnaryExpr(UnaryOperation op, const string &rhs)
	{
		string rhsUnique, rhs_type;
		prepare(rhs, rhsUnique, rhs_type);
		if(rhs_type=="i32"&&!isSymbol(rhsUnique))
		{
			int rhsValue=atoi(rhsUnique.c_str());
			switch(op)
			{
				case UO_MINUS:
					return to_string(-rhsValue);
				case UO_NOT:
					return to_string(!rhsValue);
				default :
					return "";
			}
		}
		else return "";
	}
	string Block::addBinaryExpr(const string &lhs, BinaryOperation op, const string &rhs)
	{
		if(symbolTable.isInGlobal())
			throw InvalidOperation();

		string lhs_type;
		string rhs_type;
		string lhsUnique;
		string rhsUnique;

		prepare(lhs, lhsUnique, lhs_type);
		prepare(rhs, rhsUnique, rhs_type);

		if(lhs_type!=rhs_type)
		{
			string targetType;
			if(isPointerType(lhs_type)||isPointerType(rhs_type))
				throw TypeError("TypeError: Not allow pointer type conversion "+lhs_type+" to "+rhs_type);
			// try implicit conversion
			if(lhs_type=="double"||rhs_type=="double")
				targetType="double";
			else if(lhs_type=="float"||rhs_type=="float")
				targetType="float";
			else if(lhs_type=="i32"||rhs_type=="i32")
				targetType="i32";
			else if(lhs_type=="i8"||rhs_type=="i8")
				targetType="i8";
			else
				throw TypeError("TypeError: Unknown type conversion "+lhs_type+" to "+rhs_type);

			lhsUnique=typeCasting(lhs_type, lhsUnique, targetType);
			rhsUnique=typeCasting(rhs_type, rhsUnique, targetType);
			lhs_type=rhs_type=targetType;
		}

		if(!isSymbol(lhsUnique)&&!isSymbol(rhsUnique)&&lhs_type=="i32")// If it just two numbers binary operating then just do it.
		{
			int lhsValue=atoi(lhsUnique.c_str());
			int rhsValue=atoi(rhsUnique.c_str());
			switch(op)
			{
				case BO_ADD:
					return to_string(lhsValue+rhsValue);
				case BO_SUB:
					return to_string(lhsValue-rhsValue);
				case BO_MUL:
					return to_string(lhsValue*rhsValue);
				case BO_DIV:
					return to_string(lhsValue/rhsValue);
				case BO_EQ:
					return to_string(lhsValue==rhsValue);
				case BO_NE:
					return to_string(lhsValue!=rhsValue);
				case BO_GT:
					return to_string(lhsValue>rhsValue);
				case BO_GE:
					return to_string(lhsValue>=rhsValue);
				case BO_LT:
					return to_string(lhsValue<rhsValue);
				case BO_LE:
					return to_string(lhsValue<=rhsValue);
				default:
					return "";
			}
		}
		if(lhs_type=="i32")
		{
			string outType;
			string code=genBasicOperation_i32(op, lhsUnique, rhsUnique, outType);
			string tmpVar=allocTemp(outType);
			tmpVar = tmpVar;

			addVariableDecl(stringToType(outType), tmpVar.substr(1), 0);

			m_ll+=tmpVar+" = "+code+"\n";
			return tmpVar;
		}
		if(lhs_type=="float"||lhs_type=="double")
		{
			string outType=lhs_type;
			string code=genBasicOperation_floating(op, lhsUnique, rhsUnique, outType);
			string tmpVar=allocTemp(outType);
			addVariableDecl(stringToType(outType), tmpVar.substr(1), 0);
			m_ll+=tmpVar+" = "+code+"\n";
			return tmpVar;
		}

		throw InvalidOperation("Error on binary operation");
	}
	string Block::addAssignStmt(const string &lhs, const string &rhs)
	{
		if(symbolTable.isInGlobal())
			throw InvalidOperation();

		string lhs_type;
		string lhsUnique;
		prepare(lhs, lhsUnique, lhs_type, true);

		string rhs_type;
		string rhsUnique;
		prepare(rhs, rhsUnique, rhs_type);

		// if lhs type is pointer type of rhs
		if(!isSymbol(lhsUnique))
			throw InvalidOperation();

		if(isPointerType(lhs_type))
		{
			string targetType=ref(lhs_type);
			rhsUnique=typeCasting(rhs_type, rhsUnique, targetType);
			rhs_type=targetType;
		}else if(!isTempVar(lhs_type))
		{
			string targetType=lhs_type;
			rhsUnique=typeCasting(rhs_type.substr(0,rhs_type.size()-1), rhsUnique, targetType);
			rhs_type=targetType;
		}

		if(lhs_type==rhs_type+"*")
		{
			m_ll+="store "+rhs_type+" "+rhsUnique+", "+lhs_type+" "+lhsUnique+"\n";
			m_recentData[lhsUnique]=rhsUnique;
			return lhsUnique;
		}

		if(lhs_type==rhs_type)
		{
			// This must be function parameter
			string p_addr=m_addressedParameter[lhsUnique];
			m_ll+="store "+rhs_type+" "+rhsUnique+", "+lhs_type+"* "+p_addr+"\n";
			m_recentData[lhsUnique]=rhsUnique;
			return lhsUnique;
		}

		throw TypeError("Type Error: &"+lhs+lhs_type+" to  "+rhs+rhs_type);
	}
	string Block::addFunctionCall(const string &func, const list<string> &params)
	{
		if(symbolTable.isInGlobal())//No function should call in global
			throw InvalidOperation();

		Symbol func_symbol;
		if(!symbolTable.lookUp(func, func_symbol)||func_symbol.paramTypes.front()!="DUMMY")
			throw TypeError("The symbol is not function");
		if(func_symbol.paramTypes.size()!=params.size()+1)
			throw TypeError("Number of argument does not match");

		

		auto it=func_symbol.paramTypes.cbegin();
		auto it2=params.cbegin();
		++it; //Miss the "DUMMY"
		string arg_list;
		for(;it!=func_symbol.paramTypes.cend()&&it2!=params.cend(); ++it,++it2)
		{
			string pUnique, p_type;
			prepare(*it2, pUnique, p_type);
			if(p_type!=*it)
				throw TypeError("Type Error");
			arg_list+=p_type+" "+pUnique+", ";
		}
		string tmpVar=allocTemp(func_symbol.type);
		m_ll += tmpVar+" = tail call "+func_symbol.type+" @"+func+"(";
		m_ll += (arg_list.length()>0?arg_list.substr(0, arg_list.length()-2):arg_list)+")\n";
		return tmpVar;
	}
	//Private Funcs
	string Block::getSymbolType(const string &name)
	{
		Symbol result;
		if(symbolTable.lookUp(name, result))
			return result.type;
		else
			throw UndefinedSymbol();
	}

	string Block::genBasicOperation_i32(BinaryOperation op, const string &lhs, 
		const string &rhs, string &outType)
	{
		string head;
		switch(op)
		{
			case BO_ADD:
				head="add"; outType="i32"; break;
			case BO_SUB:
				head="sub"; outType="i32"; break;
			case BO_MUL:
				head="mul"; outType="i32"; break;
			case BO_DIV:
				head="sdiv"; outType="i32"; break;
			case BO_EQ:
				head="icmp eq"; outType="i1"; break;
			case BO_NE:
				head="icmp ne"; outType="i1"; break;
			case BO_GT:
				head="icmp sgt"; outType="i1"; break;
			case BO_GE:
				head="icmp sge"; outType="i1"; break;
			case BO_LT:
				head="icmp slt"; outType="i1"; break;
			case BO_LE:
				head="icmp sle"; outType="i1"; break;

		}

		return head+" i32 "+lhs+", "+rhs;
	}

	string Block::genBasicOperation_floating(BinaryOperation op, const string &lhs, 
		const string &rhs, string &outType)
	{
		string head;
		switch(op)
		{
			case BO_ADD:
				head="fadd"; break;
			case BO_SUB:
				head="fsub";break;
			case BO_MUL:
				head="fmul"; break;
			case BO_DIV:
				head="fdiv"; break;
			case BO_EQ:
				head="fcmp oeq"; outType="i1"; break;
			case BO_NE:
				head="fcmp one"; outType="i1"; break;
			case BO_GT:
				head="fcmp ogt"; outType="i1"; break;
			case BO_GE:
				head="fcmp oge"; outType="i1"; break;
			case BO_LT:
				head="fcmp olt"; outType="i1"; break;
			case BO_LE:
				head="fcmp ole"; outType="i1"; break;
			default :
				head = "";
		}

		return head+" "+outType+" "+lhs+", "+rhs;
	}

	string Block::allocTemp(const string &type)
	{
		Symbol symbol;
		symbol.type=type;
		symbol.name=to_string(++m_tempCounter);

		//symbolTable.insert(symbol.name, symbol);
		//fprintf(stderr, "%%%s(%s) in scope %d\n", symbol.name.c_str(), type.c_str(), m_ifClause.size());
		return "%"+symbol.name;

	}
	string Block::getElementType(const string &type)
	{
		auto left = type.find_first_of("[");
		auto right = type.find_last_of("]");
		//It's shall not be empty
		if(isPointerType(type)&&left!=string::npos&&right!=string::npos)
		{
			auto mid = type.find_first_of("x");
			return type.substr(mid+2, right-mid-2);
		}else
			throw InvalidOperation("Type Error: "+type+" is not an array type");
	}

	bool Block::isPointerType(const string &type)
	{

		return type.substr(type.length()-1)=="*";
	}

	string Block::ref(const string &type)//
	{
		if(isPointerType(type))
			return type.substr(0, type.length()-1); //just get off the *
		else
			throw TypeError("TypeError: Try to reference a non pointer type value."+type);
	}


	int Block::isTempVar(const string &name)
	{
		if(name.length()>=2&&name[0]=='%'&&isdigit(name[1]))
			return atoi(name.c_str()+1);
		else
			return 0;
	}

	bool Block::isSymbol(const string &name) const
	{
		return !(isdigit(name[0])||(name[0]=='-'&&isdigit(name[1])));
	}

	string Block::loadMemory(const string &name_, const string &sub)
	{
		if(symbolTable.isInGlobal())
			throw InvalidOperation();

		if(!isSymbol(name_))
			return name_;

		string name=isTempVar(name_)?name_.substr(1):name_;

		Symbol result;
		if(symbolTable.lookUp(name, result))
		{
			string uniqueName;
			symbolTable.getUniqueName(name, uniqueName);
			//check type;
			if(isPointerType(result.type))
			{
				if(sub!="")
				{
					string &cachePtr=m_recentData[uniqueName+"+"+sub];
					string elementType=getElementType(result.type);
					if(cachePtr=="")
					{
						cachePtr=allocTemp(elementType+"*");
						m_ll+=cachePtr+"= getelementptr inbounds "+result.type+" "+uniqueName+", i32 0, i32 "+sub+"\n";
					}
					string &tmpVarName=m_recentData[uniqueName+"["+sub+"]"];
					if(tmpVarName=="")
					{
						tmpVarName=allocTemp(elementType);
						m_ll+=tmpVarName+" = load "+elementType+"* "+cachePtr+", align 4\n";
					}
					return tmpVarName;
				}else
				{
					// Use unique name!!!
					string &cache=m_recentData[uniqueName];
					if(cache!="") return cache;

					string tmpVarName=allocTemp(result.type.substr(0, result.type.length()-1));
					m_ll+=tmpVarName+" = load "+result.type+" "+uniqueName+", align 4\n";
					
					cache=tmpVarName;
					return tmpVarName;
				}
			}else if(!isTempVar(name))
			{
				// load recent data for parameter
				string &cache=m_recentData[uniqueName];
				string target=cache;

				if(cache=="")
				{
					// Load it!!
					target=m_addressedParameter[uniqueName];

					string tmpVarName=allocTemp(result.type);
					m_ll+=tmpVarName+" = load "+result.type+"* "+target+", align 4\n";
					target=tmpVarName;
				}
				cache=target;
				return target;
			}else
				throw TypeError("Type Error in LoadMemory"+name+"["+sub+"]");
		}else
			throw UndefinedSymbol("UndefinedSymbol: "+name+"["+sub+"]");
		
	}

	string Block::typeCasting(const string &type, const string &var, const string &targetType)
	{
		if(type==targetType)
			return var;
		
		cout<<"Warning : in scope"+m_funcName+"{"+var+" try to covert from "+type+" to "+targetType+"}";
		string casted=allocTemp(targetType);
		if(type=="i1"||type=="i8"||type=="i32")
		{
			m_ll+=casted+" = sitofp "+type+" "+var+" to "+targetType+"\n";
		}else if(type=="float")
		{
			if(targetType=="double")
			{
				if(isSymbol(var))
					m_ll+=casted+" = fpext "+type+" "+var+" to double\n";
			}else
				// integer in double
				m_ll+=casted+" = fptosi float "+var+" to "+targetType+"\n";
		} else if(type=="double")
		{
			if(targetType=="float")
			{
				if(isSymbol(var))
				m_ll+=casted+" = fptrunc "+type+" "+var+" to float\n";
			}else
				// integer in float 
				m_ll+=casted+" = fptosi double "+var+" to "+targetType+"\n";
		} else
			throw TypeError("TypeError: Unable to cast from "+type+" to "+targetType);
		return casted;
	}

	Type Block::stringToType(const string & in)
	{
		if(in == "") return T_VOID;
		if(in == "i8") return T_CHAR;
		if(in == "i32") return T_INT;
		if(in == "float") return T_FLOAT;
		if(in == "double") return T_DOUBLE;
		return T_VOID;
	}

	//When doing operating it will load elemnt from symbol table, and assign to temp
	void Block::prepare(const string &inName, string &outNameValue,string &outType, bool dontLoad){
		size_t niddle=inName.find_first_of("[");
		string name=inName;
		string sub;
		if(niddle!=string::npos)//it is an array type
		{
			name=inName.substr(0, niddle);
			string tmp=inName.substr(niddle+1, inName.length()-niddle-2); // 2 is for "[]"
			string sub_type;
			prepare(tmp, sub, sub_type);
			if(sub_type!="i32")
				throw TypeError("Type Error: Only i32 can be used for array subscripting");
		}

		if(isSymbol(name))
		{
			bool isTemp=isTempVar(name);
			string tmp=isTemp?name.substr(1):name;
			symbolTable.getUniqueName(tmp, outNameValue);
			Symbol outVar;
			symbolTable.lookUp(tmp, outVar);
			outType=outVar.type;

			// Check if this variable has been loaded(clang)
			Symbol dummy;
			if(!dontLoad&&!isTemp)
			{
				outNameValue=loadMemory(name);
				if(isPointerType(outType))
					outType=ref(outType);
			}
			if(dontLoad&&!isTemp&&outVar.isArray())
			{
				string &cachePtr=m_recentData[outNameValue+"+"+sub];
				outType=getElementType(outVar.type)+"*";//TIP: array in llvm is a pointer type
				if(cachePtr=="")
				{
					cachePtr=allocTemp(outType);
					m_ll+=cachePtr+"= getelementptr inbounds "+outVar.type+" "+outNameValue+", i32 0, i32 "+sub+"\n";
				}

				outNameValue=cachePtr;
			}
		}
		else{
			outNameValue=name;
			if(outNameValue.find(".")==string::npos)// To see if the constance is int of float
				outType="i32";
			else
				outType="double";
		}
	}


}//namespace