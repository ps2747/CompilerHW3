#include"TreeNode.h"
#include"TreeAnalizer.h"

#include <fstream>
#include<iostream>
#include<string>
#include<cstdlib>
#include<vector>

extern void printST();
int main()
{
	fstream file; 
    TreeAnalizer* myanalizer = new TreeAnalizer();
    file.open("main.ll", ios::out|ios::trunc);
    std::string outPut = myanalizer->printBlock();
    printST();
    std::cout<<outPut;
    file << outPut;
    return 0;
}
