#ifndef TREENODE_H
#define TREENODE_H
#include<iostream>
#include<cstdlib>
#include<string>
#include<vector>
using namespace std;

class TreeNode
{
public:
    TreeNode();//constructor()
	TreeNode(string s,TreeNode* p);//constructor
    string symbol;
	TreeNode* parent;
	vector<TreeNode> child;
};

#endif
