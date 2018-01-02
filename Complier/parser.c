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
adding: iteration-stmt  ===>  for-stmt | while-stmt | do-while-stmt
adding: for-stmt ===> 'for '( expression '; expression '; expression ') statement
adding: do-while-stmt  ===>  'do statement 'while '( expression ') ';
adding: while-stmt  ===>  while ( expression ) statement
14. expression-stmt  ===>  expression ; | ;
15. selection-stmt  ===>  if (expression ) statement |if (expression ) statement else statement
16. (delete) iteration-stmt  ===>  while ( expression ) statement
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

TreeNode * dcl_list(Boolean *ok);

/* 3 */
TreeNode * declaration(Boolean *ok);

/* 4 */
static TokenType  type_specifier(void);

/* 5 */
TreeNode * var_declaration(Boolean *ok);

/* 6 */
TreeNode * array_declaration(Boolean *ok);

/* 7 */
TreeNode * fun_declaration(Boolean *ok);

/* 8 */
TreeNode * params(Boolean *ok);

/* 9 */
TreeNode * param_list(Boolean *ok);

/* 10 */
TreeNode * param(Boolean *ok);

/* 11 */
TreeNode * compound_stmt(Boolean *ok);

/* 12   can return NULL */
TreeNode * local_dcl_list(Boolean *ok);

/* 13 */
TreeNode * local_declaration(Boolean *ok);

/* 14 can return NULL */
TreeNode * statement_list(Boolean *ok);

/* 15 */
TreeNode * statement(Boolean *ok);

/*15.5*/
TreeNode *iteration_stmt(Boolean *ok);

/* 16 */
TreeNode * selection_stmt(Boolean *ok);

/* 17 can return NULL */
TreeNode * else_part(Boolean *ok);

/* 18  */
TreeNode * while_stmt(Boolean *ok);

/* 19 */
TreeNode * do_while_stmt(Boolean *ok);

/* 20 */
TreeNode * for_stmt(Boolean *ok);

/* 22 */
TreeNode * return_stmt(Boolean *ok);

/* 24 */
TreeNode * expr_stmt(Boolean *ok);

/* 25 */
TreeNode * expression(Boolean *ok);

/* 26 */
TreeNode * comma_expr(Boolean *ok);

/* 27 */
TreeNode * assignment_expr(Boolean *ok);

/* 28 */
TreeNode * lhs(Boolean *ok);

/* 29 */
TreeNode * array_element(Boolean *ok);

/* 30 */
TreeNode * equality_expr(Boolean *ok);

/* 31 */
TreeNode * eqop(Boolean *ok);

/* 32 */
TreeNode * relational_expr(Boolean *ok);

/* 33 */
TreeNode * relop(Boolean *ok);

/* 34 */
TreeNode * additive_expr(Boolean *ok);

/* 35 */
TreeNode * addop(Boolean *ok);

/* 36 */
TreeNode * multiplicative_expr(Boolean *ok);

/* 37 */
TreeNode * mulop(Boolean *ok);

/* 38 */
TreeNode * primary_expr(Boolean *ok);

/* 39 */
TreeNode * call(Boolean *ok);

/* 40  can return NULL */
TreeNode * arg_list(Boolean *ok);





/*
declaration-list  ===>  declaration-list declaration | declaration
*/
TreeNode *declaration_list(Boolean *ok)
{
	TreeNode * t = NULL;
	TreeNode * l = NULL;
	TreeNode * r = NULL;

	while (!check(CurrentToken, FOR))
	{
		r = declaration(ok);

		if (l == NULL) { // assign the first node of declaration list
			t = l = r;
		}
		else if (r != NULL) {
			l->rSibling = r;
			r->lSibling = l;
			l = r;
		}
	}
	return t;
}

/*
declaration  ===>  var-declaration | fun-declaration | array-declaration
*/
TreeNode *declaration(Boolean *ok)
{
	TreeNode *t = NULL;
	t = var_declaration(ok);
	if (t != NULL) return t;

	t = array_declaration(ok);
	if (t != NULL) return t;

	t = fun_declaration(ok);
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
TreeNode *var_declaration(Boolean *ok)
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
			TokenType type = type_specifier();//Move one Token
			CurrentToken = reach_node(node, 3);//use three token

			t = new_dcl_node(VAR_DCL, lineNum);
			t->attr.dclAttr.type = (type == VOID ? VOID_TYPE : INT_TYPE);
			t->attr.dclAttr.name = string_clone(nextone->token->string);
			return t;
		}
	}
	return NULL;
}

/*
array-declaration  ===>  type-specifier ID  [ NUM ] ;
*/
TreeNode *array_declaration(Boolean *ok)
{
	TreeNode *t = NULL;
	int lineNum = CurrentToken->lineNum;
	TOKENNODE *node = CurrentToken;
	TOKENNODE *nextone = reach_node(CurrentToken, 1);
	TOKENNODE *nexttwo = reach_node(CurrentToken, 2);
	TOKENNODE *nextthree = reach_node(CurrentToken, 3);
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
			TokenType type = type_specifier();//move one token
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
TreeNode *fun_declaration(Boolean *ok)
{
	TreeNode *t = new_dcl_node(FUN_DCL, CurrentToken->lineNum);
	TokenType type = type_specifier();
	TOKENNODE *funID = CurrentToken;
	Boolean status;
	if (match_move(ID))
	{
		t->attr.dclAttr.type = (type = INT) ? INT_TYPE : VOID_TYPE;
		t->attr.dclAttr.name = string_clone(funID->token->string);

		if (match_move(LPAR))
		{
			t->child[0] = params(&status);
			if (match_move(RPAR) && status == TRUE)
			{
				t->child[1] = compound_stmt(&status);
				if (status == TRUE)
					return parse_good_return(t, ok);
			}
		}
	}

	return parse_bad_return(t, ok);
}

/*
params  ===>  param-list | void
check if the params are void
*/
TreeNode * params(Boolean *ok)
{
	TreeNode *t = NULL;
	TOKENNODE *old = CurrentToken;
	TOKENNODE *nextone = reach_node(CurrentToken, 1);
	if (check(old, VOID) && check(nextone, RPAR))
	{
		t = new_param_node(VOID_PARAM, nextone->lineNum);
		t->attr.dclAttr.type = VOID_TYPE;
		t->attr.dclAttr.name = string_clone(old->token->string);
		CurrentToken = reach_node(CurrentToken, 1);
		*ok = TRUE;
		return t;
	}

	return param_list(ok);
}

/*
param-list  ===>  param-list ', param | param
*/
TreeNode * param_list(Boolean *ok)
{
	TreeNode *t = NULL;
	TreeNode *last = NULL;
	TreeNode *right = NULL;
	Boolean status;
	while (!check(CurrentToken, RPAR)) {

		if (right != NULL) {
			if (!match_move(COMMA))
				return parse_bad_return(t, ok);
		}

		right = param(&status);

		if (last == NULL) {
			t = last = right;
		}
		else if (right != NULL) {
			last->rSibling = right;
			right->lSibling = last;

			last = right;
		}

		if (status == FALSE)
			return parse_bad_return(t, ok);
	}

	return parse_good_return(t, ok);
}

/*
param  ===>  type-specifier 'ID | type-specifier 'ID '[  ']
can be array, reuse the decl function above.
*/
TreeNode * param(Boolean *ok)
{
	TreeNode *t = NULL;
	TOKENNODE *nextone = reach_node(CurrentToken, 1);
	TOKENNODE *nexttwo = reach_node(CurrentToken, 2);
	TOKENNODE *nextthree = reach_node(CurrentToken, 3);
	int parameterSteps;
	TokenType type = type_specifier();

	if (type == INT || type == VOID)
	{
		ParamKind paramtype = VAR_PARAM;
		if (check(nextone, ID))
		{
			parameterSteps = 1;
			if (check(nexttwo, LBR) && check(nextthree, RBR))//check array
			{
				paramtype = ARRAY_PARAM;
				parameterSteps += 2;
			}
			t = new_param_node(paramtype, nextone->lineNum);
			t->attr.dclAttr.type = (type == VOID ? VOID_TYPE : INT_TYPE);
			t->attr.dclAttr.name = string_clone(nextone->token->string);
			CurrentToken = reach_node(CurrentToken, parameterSteps);

			return parse_good_return(t, ok);
		}
	}

	return parse_bad_return(t, ok);
}

/*
compound-stmt  ===>  { local-declarations statement-list }
*/
TreeNode * compound_stmt(Boolean *ok)
{
	Boolean status;
	TreeNode *node = new_stmt_node(CMPD_STMT, CurrentToken->lineNum);

	if (match_move(LCUR))
	{
		node->child[0] = local_dcl_list(&status);
		if (status == TRUE)
			node->child[1] = statement_list(ok);

		if (match_move(RCUR) && status == TRUE)
		{
			return parse_good_return(node, ok);
		}
	}
	return parse_bad_return(node, ok);
}

/*
   local-dcl-list  ===>  local-dcl local-dcl-list | EMPTY
   can be null
*/
TreeNode * local_dcl_list(Boolean *ok)
{
	TreeNode *t = NULL;
	Boolean status;

	t = local_declaration(&status);

	if (t == NULL) return NULL;

	TreeNode *sibling = local_dcl_list(ok);
	if (sibling != NULL) {
		t->rSibling = sibling;
		sibling->lSibling = t;
	}

	*ok = TRUE;
	return t;
}

/*
local-dcl	===>  var-dcl | array-dcl
*/
TreeNode * local_declaration(Boolean *ok)
{
	TreeNode *t = NULL;

	t = var_declaration(ok);
	if (t != NULL) {
		*ok = TRUE;
		return t;
	}

	t = array_declaration(ok);
	if (t != NULL) {
		*ok = TRUE;
		return t;
	}

	*ok = FALSE;
	return NULL;

}

/*
statement-list  ===>  statement-list statement |empty
*/
TreeNode * statement_list(Boolean *ok)
{
	TreeNode * t = NULL;
	TreeNode * l = NULL;
	TreeNode * r = NULL;
	Boolean status;
	while (check(CurrentToken, RCUR))
	{
		r = statement(&status);
		if (l = NULL)
		{
			t = l = r;
		}
		else if (r != NULL)
		{
			l->rSibling = r;
			r->lSibling = l;

			l = r;
		}
		if (status = FALSE)
		{
			return parse_bad_return(t, ok);
		}
	}
	parse_good_return(t, ok);
	return t;
}

/*
statement  ===>  expression-stmt | compound-stmt | selection-stmt | iteration-stmt | return-stmt
*/
TreeNode * statement(Boolean *ok)
{
	TreeNode *t = NULL;
	Boolean status;

	switch (CurrentToken->token->type)
	{
	case LCUR: t = compound_stmt(&status); break;
	case IF: t = selection_stmt(&status); break;
	case RETURN: t = return_stmt(&status); break;
	case SEMI: t = new_stmt_node(NULL_STMT, CurrentToken->lineNum); break;
	case ID: t = expr_stmt(&status); break;
	case FOR:case WHILE:case DO: t = iteration_stmt(&status); break;
	default:
		break;
	}
	if (t != NULL && status == TRUE)
		return parse_good_return(t, ok);
	else
		return parse_bad_return(t, ok);
}
/*
iteration-stmt  ===>  for-stmt | while-stmt | do-while-stmt
*/
TreeNode *iteration_stmt(Boolean *ok)
{
	TreeNode *t = NULL;
	Boolean status;
	switch (CurrentToken->token->type)
	{
	case WHILE: t = while_stmt(&status); break;
	case DO: t = do_while_stmt(&status); break;
	case FOR: t = for_stmt(&status); break;
	default:
		break;
	}
	if (t != NULL && status == TRUE)
		return parse_good_return(t, ok);
	else
		return parse_bad_return(t, ok);
}

/*
 selection-stmt  ===>  if (expression ) statement |if (expression ) statement else statement
*/
TreeNode * selection_stmt(Boolean *ok)
{
	TreeNode *t = NULL;
	t = new_stmt_node(SLCT_STMT, CurrentToken->lineNum);
	Boolean status;

	if (match_move(IF))
	{
		if (match_move(LPAR))
		{
			t->child[0] = expression(&status);
		}
		if (match_move(RPAR) && status == TRUE)
		{
			t->child[1] = statement(&status);
		}
		if (status == TRUE&&check(CurrentToken, ELSE))//else statement
		{
			t->child[2] = else_part(&status);

		}
		else if (status == TRUE)
		{
			t->child[2] = NULL;
			return parse_good_return(t, ok);
		}

	}

	return parse_bad_return(t, ok);
}

/* else ===> else | statement  */
TreeNode * else_part(Boolean *ok)
{
	TreeNode *t = NULL;
	Boolean status;
	match_move(ELSE);
	t = statement(&status);
	if (status)
		return parse_good_return(t, ok);
	else
		return parse_bad_return(t, ok);
}

/*
 while-stmt  ===>  while ( expression ) statement
*/
TreeNode * while_stmt(Boolean *ok)
{
	TreeNode *t = NULL;
	t = new_stmt_node(WHILE_STMT, CurrentToken->lineNum);
	Boolean status;
	if (match_move(WHILE))
	{
		if (match_move(LPAR))
		{
			t->child[0] = expression(&status);
			if (match_move(RPAR) && status)
			{
				t->child[1] = statement(&status);
				if (status)
					return parse_good_return(t, ok);
			}
		}
	}
	/* failure */
	return parse_bad_return(t, ok);

	return t;
}

/*
do-while-stmt  ===>  'do statement 'while '( expression ') ';
*/
TreeNode * do_while_stmt(Boolean *ok)
{
	TreeNode *t = NULL;
	t = new_stmt_node(DO_WHILE_STMT, CurrentToken->lineNum);
	Boolean status;

	if (match_move(DO))
	{
		t->child[0] = statement(&status);
		if (match_move(WHILE) && status)
			if (match_move(LPAR))
			{
				t->child[1] = expression(&status);
				if (match_move(RPAR) && status)
					if (match_move(SEMI))
						return parse_good_return(t, ok);
			}
	}
	return parse_bad_return(t, ok);
}

/*
for-stmt ===> 'for '( expression '; expression '; expression ') statement
*/
TreeNode * for_stmt(Boolean *ok)
{
	TreeNode *t = NULL;
	t = new_stmt_node(FOR_STMT, CurrentToken->lineNum);
	Boolean status;
	if (match_move(FOR))
		if (match_move(LPAR))
		{
			t->child[0] = expression(&status);
			if (match_move(SEMI) && status)
			{
				t->child[1] = expression(&status);
				if (match_move(SEMI))
				{
					t->child[2] = expression(&status);
					if (match_move(RPAR))
					{
						t->child[3] = statement(&status);
						return parse_good_return(t, ok);
					}
				}
			}
		}

	return parse_bad_return(t, ok);
}


/* 22 */
TreeNode * return_stmt(Boolean *ok)
{
	TreeNode *t = NULL;
	t = new_stmt_node(RTN_STMT, CurrentToken->lineNum);
	Boolean status;
	if (match_move(RETURN))
	{
		t->child[0] = expression(&status);
		if (match_move(SEMI) && status)
			return parse_good_return(t, ok);
	}

	return parse_bad_return(t, ok);
}


/* 24 */
TreeNode * expr_stmt(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/* 25 */
TreeNode * expression(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/* 26 */
TreeNode * comma_expr(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/* 27 */
TreeNode * assignment_expr(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/* 28 */
TreeNode * lhs(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/* 29 */
TreeNode * array_element(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/* 30 */
TreeNode * equality_expr(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}


/* 32 */
TreeNode * relational_expr(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/*
relop  ===>  <= | < | > | >= | == | !=
*/
TreeNode * relop(Boolean *ok)
{
	TreeNode *t = NULL;

	if (check(CurrentToken, LT) || check(CurrentToken, LTE)
		|| check(CurrentToken, GT) || check(CurrentToken, GTE)
		|| check(CurrentToken, EQ) || check(CurrentToken, NEQ))
	{
		t = new_expr_node(OP_EXPR, CurrentToken->lineNum);
		t->attr.exprAttr.op = CurrentToken->token->type;

		return parse_good_return(t, ok);
	}

	return parse_bad_return(NULL, ok);
}

/* 34 */
TreeNode * additive_expr(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/*
addop  ===>  + | -
*/
TreeNode * addop(Boolean *ok)
{
	TreeNode *t = NULL;
	if (check(CurrentToken, PLUS) || check(CurrentToken, MINUS)) {
		t = new_expr_node(OP_EXPR, CurrentToken->lineNum);
		t->attr.exprAttr.op = CurrentToken->token->type;

		return parse_good_return(t, ok);
	}

	return parse_bad_return(NULL, ok);
}

/* 36 */
TreeNode * multiplicative_expr(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/*
mulop  ===>  * | /
*/
TreeNode * mulop(Boolean *ok)
{
	TreeNode *t = NULL;
	if (check(CurrentToken, MULTP) || check(CurrentToken, OVER)) {
		t = new_expr_node(OP_EXPR, CurrentToken->lineNum);
		t->attr.exprAttr.op = CurrentToken->token->type;

		return parse_good_return(t, ok);
	}

	return parse_bad_return(NULL, ok);
}

/* 38 */
TreeNode * primary_expr(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/* 39 */
TreeNode * call(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

/* 40  can return NULL */
TreeNode * arg_list(Boolean *ok)
{
	TreeNode *t = NULL;
	return t;
}

TreeNode *parse()
{
	TreeNode *t;
	CurrentToken = theTokenList;
	Boolean *state = TRUE;
	t = declaration_list(&state);
	return t;
}