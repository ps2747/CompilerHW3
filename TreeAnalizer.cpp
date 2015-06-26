#include "TreeAnalizer.h"
#include "TreeNode.h"
#include<iostream>
#include<cstdlib>
#include<vector>
#include<fstream>
#include<list>
using namespace std;
using namespace ToyLLGen;

Block *current = new Block();

Block *parent = current;

Block *global = current;

Type Type_trans (string inType)
{
	if(inType == "int") return T_INT;
	if(inType == "char") return T_CHAR;
	if(inType == "double") return T_DOUBLE;
	if(inType == "float") return T_FLOAT;
	if(inType == "void") return T_VOID;
	else return T_VOID;

}

TreeAnalizer::TreeAnalizer()
{
	load_tree();
	dft(&root);
	Compile(root.child[0].child[0]);
}

void TreeAnalizer::Compile(TreeNode &node)
{
	DeclList(node.child[0]);    
}

void TreeAnalizer::DeclList(TreeNode &node)
{
	if(node.child[0].symbol == "epsilon")
	{   
	    return;
	}
	DeclList_(node.child[0]);
	DeclList(node.child[1]);
}

void TreeAnalizer::DeclList_(TreeNode &node)
{
	enum Type type_value = Type_trans(node.child[0].child[0].symbol);
	string id_value = node.child[1].child[0].symbol;
	Decl(type_value,id_value,node.child[2]);
}

void TreeAnalizer::Decl(enum Type type_value,string id_value,TreeNode &node)
{
	if(node.child[0].symbol == "VarDecl'")
	{
		VarDecl_(type_value,id_value,node.child[0]);
	}
	else if(node.child[0].symbol == "FunDecl")
	{
		FunDecl(type_value,id_value,node.child[0]);
	}
}
void TreeAnalizer::VarDecl(TreeNode &node)
{
	enum Type type_value = Type_trans(node.child[0].child[0].symbol);
	string id_value = node.child[1].child[0].symbol;
	VarDecl_(type_value,id_value,node.child[2]);
}

void TreeAnalizer::VarDecl_(enum Type type_value,string id_value,TreeNode &node)
{
	if(node.child[0].symbol == ";")
	{
			
			current->addVariableDecl(type_value,id_value);
		
		//cout << type_value << " " << id_value << ";" << endl; 
	}
	else if(node.child[0].symbol == "[")
	{
			current->addVariableDecl(type_value,id_value,stoi(node.child[1].child[0].symbol));

			//gen -> Type Id [ num ];
		
		//cout << type_value << " " << id_value << "[" << node.child[1].symbol <<"]"<<";" << endl; 
	
	}
}
void TreeAnalizer::FunDecl(enum Type type_value,string id_value,TreeNode& node)	
{
	list<pair<enum Type,string> > param = ParamDeclList(node.child[1]);
	
	/*
	return type -> list<pair<type,id>>
	*/
	for (list <pair<enum Type,string> >::iterator i = param.begin(); i != param.end (); i++)
		cout << " Type: "<< (*i).first << " id: " << (*i).second << endl;
	
	FunDecl_(type_value, id_value, param, node.child[3]);
}

void TreeAnalizer::FunDecl_(enum Type type_value,string id_value,list<pair<enum Type,string> >param,TreeNode &node)
{
	if(node.child[0].symbol==";")
	{
		return;
	}
	
	current = new Block(BT_FUNC, type_value, id_value,current, current->m_tempCounter);
	current-> asFuncBlock(param);
	
	Block_(node);
	
	string funcEnd = current->endFuncBlock();
	current = current->getParentBlock();
	current->catLL(funcEnd);
	
}
void TreeAnalizer::Block_(TreeNode &node)
{
	VarDeclList(node.child[1]);
	StmtList(node.child[2]);
}

void TreeAnalizer::VarDeclList(TreeNode &node)
{
	if(node.child[0].symbol == "epsilon")
		return;
	VarDecl(node.child[0]);
	VarDeclList(node.child[1]);
}


list<pair<enum Type,string> > TreeAnalizer::ParamDeclList(TreeNode &node)
{
	if(node.child[0].symbol=="epsilon")
		return {};
	return ParamDeclListTail(node.child[0]);
}

list<pair<Type,string> >  TreeAnalizer::ParamDeclListTail(TreeNode &node)
{
	list<pair<enum Type, string > > result;
	result.push_back(ParamDecl(node.child[0]));
	const list<pair<enum Type, string > > &result2=ParamDeclListTail_(node.child[1]);
	result.insert(result.end(), result2.begin(), result2.end());
	return result;
}

pair<enum Type,string> TreeAnalizer::ParamDecl(TreeNode &node)
{
	enum Type type=Type_trans(node.child[0].child[0].symbol);
	string id= node.child[1].child[0].symbol;
	return {type, id};
}

list<pair<enum Type, string > > TreeAnalizer::ParamDeclListTail_(TreeNode &node)
{
	if(node.child[0].symbol=="epsilon")
		return {};
	return ParamDeclListTail(node.child[1]);
}

void TreeAnalizer::StmtList(TreeNode &node)
{
	Stmt(node.child[0]);
	StmtList_(node.child[1]);
}
void TreeAnalizer::StmtList_(TreeNode &node)
{
	if(node.child[0].symbol == "epsilon")
		return ;
	StmtList(node.child[0]);
}
void TreeAnalizer::Stmt(TreeNode &node)
{
	if(node.child[0].symbol=="Expr")
	{
		Expr(node.child[0]);
	}
	if(node.child[0].symbol=="return")
	{
		float r_value = Expr(node.child[1]);
		
		//llg.addReturnStmt(ExprHelper(node.children[1]));
		
	}
	if(node.child[0].symbol=="break")
	{
		
				
	}
	if(node.child[0].symbol=="if")
	{
		float cond=Expr(node.child[2]);
		if(cond>=1)
		{
			current = new Block(BT_IF, T_VOID, "if", current, current->m_tempCounter);
			current->asIfBlock(to_string(cond));
			/*
			llg.beginIfClause(cond);
			*/
			Stmt(node.child[4]);
			/*
			llg.endIfClause();
			*/
			string ifEndLL = current->endIfBlock();
			current = current->getParentBlock();
			current->catLL(ifEndLL);
		}
		else
		{
			/*
			llg.beginElseClause(cond);
			*/
			Stmt(node.child[6]);
			/*
			llg.endElseClause();
			*/
		}
	}

	if(node.child[0].symbol=="while")
	{
		/*
		llg.beginWhileCondEvalution();
		*/
		float cond=Expr(node.child[2]);
		/*
		llg.beginWhile(cond);
		*/
		Stmt(node.child[4]);
		/*
		llg.endWhile();
		*/
	}

	if(node.child[0].symbol=="Block")
	{
		Block_(node.child[0]);
	}
	
	if(node.child[0].symbol=="print")
	{
		current->print(node.child[1].child[0].symbol);
		/*
		llg -> print node.child[1]
		*/
	}
	
}

float TreeAnalizer::Expr(TreeNode &node)
{
	return 0;
}

void TreeAnalizer::load_tree()
{
	ifstream ifs("tree.txt"); 
	string symbol;
	int label;
	int tmp = 0;
	TreeNode *now = &root;
	while (ifs >> label) 
	{
	    ifs >> symbol;
	    while(label<=tmp)
	    {
			now = now->parent;
			tmp --;
	    }
	    now->child.push_back(TreeNode(symbol,now));
	    now = &now->child.back();
	    tmp = label;
	}
	ifs.close();
}

void TreeAnalizer::dft(TreeNode* tmp)
{
    if(!tmp->child.empty())
    {
	for(int i=0;i<tmp->child.size();i++)
	{
		cout << tmp->child[i].symbol << endl;
	    dft(&tmp->child[i]);
	    
	}
    }
}

string TreeAnalizer::printBlock()
{
	return global ->endBlock();
}
