#include"libs.h"
#include"parse.h"
#include"scan.h"

TokenType CurrentTokenType;
PTOKENNODE CurrentToken;
PTOKENNODE NextToken;

void getToken()
{
	CurrentTokenType = NextToken->type;
	CurrentToken = NextToken;
	NextToken = NextToken->next;
}

TreeNode *declaration_list()
{
	TreeNode *t;

	return t;
}

TreeNode *Parse(void)
{
    TreeNode *t;

    getToken();
    t = declaration_list();
    if (CurrentToken != END)
	//syntaxError("Unexpected symbol at end of file\n");

    /* t points to the fully-constructed syntax tree */
    return t;
}