#include"TreeNode.h"
#include"TreeAnalizer.h"
#include<iostream>
#include<string>
#include<cstdlib>
#include<vector>
int main()
{
    TreeAnalizer* myanalizer = new TreeAnalizer();
    std::cout<<myanalizer->printBlock();
    return 0;
}
