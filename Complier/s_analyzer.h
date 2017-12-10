/****************************************************
File: s_analyzer.h
Semantic analyzer interface for TINY compiler
Compiler Construction: Principles and Practice
Kenneth C. Louden
Programming by Zhiyao Liang
MUST FIT FALL 2016
***************************************************/

/* implements a semantic analyzer, according to the interface analyzer.h
*/

#ifndef _S_ANALYZER_H_
#define _S_ANALYZER_H_

//#include "symbol_table.h"
#include "analyzer.h"

// the symbol table on the top level.
//extern SymbTab * symbolTable;

/* Function buildSymtab constructs the symbol
* table by preorder traversal of the syntax tree
*/
//void build_symb_tab(TreeNode *);

/* Procedure typeCheck performs type checking
* by a postorder syntax tree traversal
*/
//void type_check(TreeNode *);

extern Boolean A_debugAnalyzer;

Analyzer * new_s_analyzer(TreeNode * parseTree);

#endif
