#include<iostream>
#include<cstdlib>
#include<string>
#include"TreeNode.h"
TreeNode::TreeNode()
{
    symbol = "";
	parent = NULL;
	child.clear();
}

TreeNode::TreeNode(string s,TreeNode* p)
{
    symbol = s;
	parent = p;
	child.clear();
}

