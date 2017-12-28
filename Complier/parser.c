#include"libs.h"
#include"parse.h"
#include"scan.h"
#include"util.h"
#include"globals.h"
#include"parse_util.h"
/*
1.  program  ===>  declaration-list
2.  declaration-list  ===>  declaration-list declaration | declaration
3.  declaration  ===>  var-declaration | fun-declaration
4.  var-declaration  ===>  type-specifier ID ; | type-specifier ID [ NUM ] ;
5.  type-specifier  ===>  int | void
6.  fun-declaration  ===>  type-specifier ID ( params ) compound-stmt
7.  params  ===>  param-list | void
8.  param-list  ===>  param-list , param | param
9.  param  ===>  type-specifier ID | type-specifier ID [ ]
10. compound-stmt  ===>  { local-declarations statement-list }
11. local-declarations  ===>  local-declarations var-declaration | empty
12. statement-list  ===>  statement-list statement |empty
13. statement  ===>  expression-stmt | compound-stmt | selection-stmt | iteration-stmt | return-stmt
14. expression-stmt  ===>  expression ; | ;
15. selection-stmt  ===>  if (expression ) statement |if (expression ) statement else statement
16. iteration-stmt  ===>  while ( expression ) statement
17. return-stmt  ===>  return ; | return expression ;
18. expression  ===>  var = expression | simple-exprssion
19. var  ===>  ID | ID [expression]
20. simple-expression  ===>  additive-expression relop additive-expression | additive-expression
21. relop  ===>  <= | < | > | >= | == | !=
22. additive-expression  ===>  additive-expression addop term | term
23. addop  ===>  + | -
24. term  ===>  term mulop factor | factor
25. mulop  ===>  * | /
26. factor  ===>  ( expression ) | var | call | NUM
27. call  ===>  ID ( args )
28. args  ===>  arg-list | empty
29. arg-list  ===>  arg-list , expression  | expression
*/

TokenType CurrentTokenType;
TOKENNODE *CurrentToken;

void getToken();
void match(TokenType expected);

TreeNode * dcl_list();

/* 3 */
TreeNode * declaration();

/* 4 */
static TokenType  type_specifier(void);

/* 5 */
TreeNode * var_dcl();

/* 6 */
TreeNode * array_dcl();

/* 7 */
TreeNode * fun_dcl();

/* 8 */
TreeNode * params();

/* 9 */
TreeNode * param_list();

/* 10 */
TreeNode * param();

/* 11 */
TreeNode * compound_stmt();

/* 12   can return NULL */
TreeNode * local_dcl_list();

/* 13 */
TreeNode * local_dcl();

/* 14 can return NULL */
TreeNode * stmt_list();

/* 15 */
TreeNode * statement();

/* 16 */
TreeNode * selection_stmt();

/* 17 can return NULL */
TreeNode * else_part();

/* 18  */
TreeNode * while_stmt();

/* 19 */
TreeNode * do_while_stmt();

/* 20 */
TreeNode * for_stmt();

/* 21 Can return NULL */
TreeNode * expr_or_empty();

/* 22 */
TreeNode * return_stmt();

/* 23 */
TreeNode * null_stmt();

/* 24 */
TreeNode * expr_stmt();

/* 25 */
TreeNode * expression();

/* 26 */
TreeNode * comma_expr();

/* 27 */
TreeNode * assignment_expr();

/* 28 */
TreeNode * lhs();

/* 29 */
TreeNode * array_element();

/* 30 */
TreeNode * equality_expr();

/* 31 */
TreeNode * eqop();

/* 32 */
TreeNode * relational_expr();

/* 33 */
TreeNode * relop();

/* 34 */
TreeNode * additive_expr();

/* 35 */
TreeNode * addop();

/* 36 */
TreeNode * multiplicative_expr();

/* 37 */
TreeNode * mulop();

/* 38 */
TreeNode * primary_expr();

/* 39 */
TreeNode * call();

/* 40  can return NULL */
TreeNode * arg_list();





/*
declaration-list  ===>  declaration-list declaration | declaration
*/
TreeNode *declaration_list()
{
	TreeNode * t = NULL;
	TreeNode * l = NULL;
	TreeNode * r = NULL;

	r = declaration();

	if (l == NULL) { // assign the first node of declaration list
		t = l = r;
	}
	else if (r != NULL) {
		l->rSibling = r;
		r->lSibling = l;
		l = r;
	}

	return t;
}

/*
declaration  ===>  var-declaration | fun-declaration | array-declaration
*/
TreeNode *declaration()
{
	TreeNode *t = NULL;
	t = var_dcl();
	if (t != NULL) return t;

	t = array_dcl();
	if (t != NULL) return t;

	t = fun_dcl();
	if (t != NULL) return t;

	return t;
}

/*
type-specifier  ===>  int | void
*/
TokenType  type_specifier(void)
{
	if (check(CurrentToken, INT)) {
		getToken();
		return INT;
	}
	if (check(CurrentToken, VOID)) {
		getToken();
		return VOID;
	}
}

/*
var-declaration  ===>  type-specifier ID ;
*/
TreeNode *var_dcl()
{
	TreeNode *t = NULL;
	int lineNum = CurrentToken->lineNum;
	TOKENNODE *node = CurrentToken;
	TOKENNODE *nextone = reach_node(CurrentToken, 1);
	TOKENNODE *nexttwo = reach_node(CurrentToken, 2);
	if (check(CurrentToken, INT) || check(CurrentToken, VOID))
	{
		if (check(nextone, ID) && check(nexttwo, SEMI))
		{
			TokenType type = type_specifier();
			CurrentToken = reach_node(node, 3);//use three token

			t = new_dcl_node(VAR_DCL, lineNum);
			t->attr.dclAttr.type = (type == VOID ? VOID_TYPE : INT_TYPE);
			t->attr.dclAttr.name = string_clone(nextone->token->string);
		}
	}
	return t;
}

/*
array-declaration  ===>  type-specifier ID  [ NUM ] ;
*/
TreeNode *array_dcl()
{
	TreeNode *t = NULL;
	int lineNum = CurrentToken->lineNum;
	TOKENNODE *node = CurrentToken;
	TOKENNODE *nextone  = reach_node(CurrentToken, 1);
	TOKENNODE *nexttwo  = reach_node(CurrentToken, 2);
	TOKENNODE *nextthree  = reach_node(CurrentToken, 3);
	TOKENNODE *nextfour = reach_node(CurrentToken, 4);
	TOKENNODE *nextfive = reach_node(CurrentToken, 5);
	if (check(CurrentToken, INT) || check(CurrentToken, VOID))
	{
		if (check(nextone, ID) &&
			check(nexttwo, LBR) &&
			check(nextthree, NUMBER) &&
			check(nextfour, RBR) &&
			check(nextfive, SEMI))
		{
			TokenType type = type_specifier();
			CurrentToken = reach_node(node, 6);//use 6 token.
			t = new_dcl_node(ARRAY_DCL, lineNum);
			t->attr.dclAttr.type = (type == VOID ? VOID_TYPE : INT_TYPE);
			t->attr.dclAttr.size = atoi(nextthree->token->string);
			t->attr.dclAttr.name = string_clone(nextone->token->string);

				return t;
		}
	}
	return NULL;
}

/*
fun-declaration  ===>  type-specifier ID ( params ) compound-stmt
*/
TreeNode *fun_dcl()
{
	TreeNode *t = NULL;
	return t;
}

/* 8 */
TreeNode * params()
{
	TreeNode *t = NULL;
	return t;
}

/* 9 */
TreeNode * param_list()
{
	TreeNode *t = NULL;
	return t;
}

/* 10 */
TreeNode * param()
{
	TreeNode *t = NULL;
	return t;
}

/* 11 */
TreeNode * compound_stmt()
{
	TreeNode *t = NULL;
	return t;
}

/* 12   can return NULL */
TreeNode * local_dcl_list()
{
	TreeNode *t = NULL;
	return t;
}

/* 13 */
TreeNode * local_dcl()
{
	TreeNode *t = NULL;
	return t;
}

/* 14 can return NULL */
TreeNode * stmt_list()
{
	TreeNode *t = NULL;
	return t;
}

/* 15 */
TreeNode * statement()
{
	TreeNode *t = NULL;
	return t;
}

/* 16 */
TreeNode * selection_stmt()
{
	TreeNode *t = NULL;
	return t;
}

/* 17 can return NULL */
TreeNode * else_part()
{
	TreeNode *t = NULL;
	return t;
}

/* 18  */
TreeNode * while_stmt()
{
	TreeNode *t = NULL;
	return t;
}

/* 19 */
TreeNode * do_while_stmt()
{
	TreeNode *t = NULL;
	return t;
}

/* 20 */
TreeNode * for_stmt()
{
	TreeNode *t = NULL;
	return t;
}

/* 21 Can return NULL */
TreeNode * expr_or_empty()
{
	TreeNode *t = NULL;
	return t;
}

/* 22 */
TreeNode * return_stmt()
{
	TreeNode *t = NULL;
	return t;
}

/* 23 */
TreeNode * null_stmt()
{
	TreeNode *t = NULL;
	return t;
}

/* 24 */
TreeNode * expr_stmt()
{
	TreeNode *t = NULL;
	return t;
}

/* 25 */
TreeNode * expression()
{
	TreeNode *t = NULL;
	return t;
}

/* 26 */
TreeNode * comma_expr()
{
	TreeNode *t = NULL;
	return t;
}

/* 27 */
TreeNode * assignment_expr()
{
	TreeNode *t = NULL;
	return t;
}

/* 28 */
TreeNode * lhs()
{
	TreeNode *t = NULL;
	return t;
}

/* 29 */
TreeNode * array_element()
{
	TreeNode *t = NULL;
	return t;
}

/* 30 */
TreeNode * equality_expr()
{
	TreeNode *t = NULL;
	return t;
}

/* 31 */
TreeNode * eqop()
{
	TreeNode *t = NULL;
	return t;
}

/* 32 */
TreeNode * relational_expr()
{
	TreeNode *t = NULL;
	return t;
}

/* 33 */
TreeNode * relop()
{
	TreeNode *t = NULL;
	return t;
}

/* 34 */
TreeNode * additive_expr()
{
	TreeNode *t = NULL;
	return t;
}

/* 35 */
TreeNode * addop()
{
	TreeNode *t = NULL;
	return t;
}

/* 36 */
TreeNode * multiplicative_expr()
{
	TreeNode *t = NULL;
	return t;
}

/* 37 */
TreeNode * mulop()
{
	TreeNode *t = NULL;
	return t;
}

/* 38 */
TreeNode * primary_expr()
{
	TreeNode *t = NULL;
	return t;
}

/* 39 */
TreeNode * call()
{
	TreeNode *t = NULL;
	return t;
}

/* 40  can return NULL */
TreeNode * arg_list()
{
	TreeNode *t = NULL;
	return t;
}

TreeNode *parse()
{
	TreeNode *t;
	CurrentToken = theTokenList;
	
	t = declaration_list();
	return t;
}