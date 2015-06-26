#ifndef TREEANALIZER_H
#define TREEANALIZER_H
#include"ll_code_gen.h"
#include"TreeNode.h"
#include<iostream>
#include<cstdlib>
#include<vector>
#include<list>
using namespace std;
using namespace ToyLLGen;
class TreeAnalizer
{
public:
    TreeAnalizer();//constructor
    void load_tree();
    void dft(TreeNode *tmp);//print tree func

	void Compile(TreeNode &node);
	void DeclList(TreeNode &node);
	void DeclList_(TreeNode &node);
	void Decl(Type type_value,string id_value,TreeNode &node);
	void VarDecl(TreeNode &node);
	void VarDecl_(Type type_value,string id_value,TreeNode &node);
	void FunDecl(Type type_value,string id_value,TreeNode &node);
	void FunDecl_(Type type_value,string id_vlaue,list<pair< Type,string> >param,TreeNode &node);
	void VarDeclList(TreeNode &node);
	list<pair <Type,string> > ParamDeclList(TreeNode &node);
	list<pair <Type,string> >  ParamDeclListTail(TreeNode &node);
	list<pair<Type, string > > ParamDeclListTail_(TreeNode &node);
	pair<Type,string>  ParamDecl(TreeNode &node);
	void Block_(TreeNode &node);
	void Type(TreeNode &node);
	void StmtList(TreeNode &node);
	void StmtList_(TreeNode &node);
	void Stmt(TreeNode &node);
	float Expr(TreeNode &node);
	void ExprIdTail(TreeNode &node);
	void ExprArrayTail(TreeNode &node);
	void Expr_(TreeNode &node);
	void ExprList(TreeNode &node);
	void ExprListTail(TreeNode &node);
	void ExprListTail_(TreeNode &node);
	string printBlock();
	
private:
    TreeNode root;
};

#endif
