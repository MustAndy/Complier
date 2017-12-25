#include"libs.h"
#include"scan.h"

void print_token_list(PTOKENNODE);
FILE *InputFile;
Boolean IfComments = FALSE;

void CreateListTail(PTOKENNODE *theTokenList)
{
	PTOKENNODE ptail, pnew;
	*theTokenList = ptail = pnew = (PTOKENNODE)malloc(sizeof(struct TkNd));
	ptail->next = pnew;
	ptail = pnew;
	ptail->next = NULL;

}

void InsertElem(PTOKENNODE *list, int i, char Token[], TokenType TokenType, int LineNumber, char state[])
{
	int j;
	PTOKENNODE p, s;
	p = *list;
	j = 1;
	while (p&&j < i)
	{
		p = p->next;
		++j;
	}
	s = (PTOKENNODE)malloc(sizeof(struct TkNd));
	s->Token = (char*)malloc(sizeof(char));
	strcpy(s->Token, Token);
	s->type = TokenType;
	s->lineNum = LineNumber;
	strcpy(s->state, state);
	s->next = p->next;
	p->next = s;
}

void print_token_list(PTOKENNODE list)
{
	int j = 1;
	while (list->next != NULL)
	{
		printf("j=%-2d", j);
		printf("   ");
		printf("%-6s %-3d %-4d %-5s\n", list->next->Token, list->next->type, list->next->lineNum, list->next->state);
		list = list->next;
		++j;
	}
}

int GetWordsTobuffer(char Buf[300], char buffer[400][20], int m)
{
	while ((fgets(Buf, 256, InputFile)) != NULL)
	{
		int n = 0;

		for (int i = 0; i < strlen(Buf); i++)
		{

			char c = Buf[i];
			//minus
			if (c == 45 || c == 43 || c == 40 || c == 41 || c == 59 || c == 91 || c == 93 || c == 123 || c == 125 || c == 44)
			{
				m++; n = 0;
				buffer[m][n] = c;
				m++; n = 0;
			}
			else if (c == 47 || c == 42)
			{
				if (Buf[i + 1] == 42 || Buf[i + 1] == 47)
				{
					m++; n = 0;
					buffer[m][n] = c;
					buffer[m][n + 1] = Buf[i + 1];
					m++; n = 0; i++;
				}
				else
				{
					m++; n = 0;
					buffer[m][n] = c;
					m++; n = 0;
				}
			}
			//word
			else if ((c < 123 && c>96) || (c > 64 && c < 90))
			{
				buffer[m][n] = c;
				n++;

			}
			//= or == or < <= > >=
			else if (c == 61 || c == 60 || c == 62)
			{
				if (Buf[i + 1] == 61)
				{
					m++; n = 0;
					buffer[m][n] = c;
					buffer[m][n + 1] = Buf[i + 1];
					m++; n = 0; i++;
				}
				else
				{
					m++; n = 0;
					buffer[m][n] = c;
					m++; n = 0;
				}
			}
			//!=
			else if (c == 33 && Buf[i + 1] == 61)
			{
				m++; n = 0;
				buffer[m][n] = c;
				buffer[m][n + 1] = Buf[i + 1];
				m++; n = 0; i++;
			}

			else if (c == 32)
			{
				m++; n = 0;

			}
			else if (c > 47 && c < 58)
			{
				buffer[m][n] = c;
				n++;
			}
			else
			{
				m++; n = 0;
				buffer[m][n] = c;
				m++; n = 0;
			}

		}
	}
	return m;
}

void InsertIntoLinkList(char buffer[400][20], int m, PTOKENNODE list)
{
	int TokenCounter = 1;
	int LineNumber = 1;
	int j;
	for (j = 0; j <= m; j++)
	{
		if (buffer[j][0] == 0)
		{
			continue;
		}
		else if (buffer[j][0] == 47 && buffer[j][1] == 42)
		{
						do
			{
				++j;
				if (buffer[j][0] == 10)
				{
					++LineNumber;
				}
			} while (buffer[j][0] != 42 && buffer[j][1] != 47);
		}
		else if (buffer[j][0] == 60)
		{
			if (buffer[j][1] == 61)
			{
				InsertElem(&list, TokenCounter, buffer[j], LTE, LineNumber, "");
				TokenCounter++;
			}
			else if (buffer[j][1] != 61)
			{
				InsertElem(&list, TokenCounter, buffer[j], LT, LineNumber, "");
				TokenCounter++;
			}
		}
		else if (buffer[j][0] == 62)
		{
			if (buffer[j][1] == 61)
			{
				InsertElem(&list, TokenCounter, buffer[j], GTE, LineNumber, "");
				TokenCounter++;
			}
			else if (buffer[j][1] != 61)
			{
				InsertElem(&list, TokenCounter, buffer[j], GT, LineNumber, "");
				TokenCounter++;
			}
		}
		else if (buffer[j][0] == 33)
		{
			if (buffer[j][1] == 61)
			{
				InsertElem(&list, TokenCounter, buffer[j], NEQ, LineNumber, "");
				TokenCounter++;
			}
		}
		else if (buffer[j][0] == 43)
		{
			InsertElem(&list, TokenCounter, buffer[j], PLUS, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] ==123 )
		{
InsertElem(&list, TokenCounter, buffer[j], LCUR, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] ==125 )
		{
InsertElem(&list, TokenCounter, buffer[j], RCUR, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] ==91 )
		{
InsertElem(&list, TokenCounter, buffer[j], LBR, LineNumber, "");
			TokenCounter++;
		}
				else if (buffer[j][0] ==93 )
		{
InsertElem(&list, TokenCounter, buffer[j], RBR, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] == 45)
		{
			InsertElem(&list, TokenCounter, buffer[j], MINUS, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] == 42)
		{
			InsertElem(&list, TokenCounter, buffer[j], MULTP, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] == 47)
		{
			InsertElem(&list, TokenCounter, buffer[j], OVER, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] == 59)
		{
			InsertElem(&list, TokenCounter, buffer[j], SEMI, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] == 10)
		{
			LineNumber++;
		}
		else if (buffer[j][0] == 61)
		{
			if (buffer[j][1] == 61)
			{
				InsertElem(&list, TokenCounter, buffer[j], EQ, LineNumber, "");
				TokenCounter++;
			}
			else if (buffer[j][1] != 61)
			{
				InsertElem(&list, TokenCounter, buffer[j], ASSIGN, LineNumber, "");
				TokenCounter++;
			}
		}
		else if (buffer[j][0] == 41)
		{
			InsertElem(&list, TokenCounter, buffer[j], RPAR, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] == 40)
		{
			InsertElem(&list, TokenCounter, buffer[j], LPAR, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] == 44)
		{
			InsertElem(&list, TokenCounter, buffer[j], COMMA, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] > 47 && buffer[j][0] < 58)
		{
			InsertElem(&list, TokenCounter, buffer[j], NUMBER, LineNumber, "");
			TokenCounter++;
		}
		//IF, ELSE, INT, RETURN, VOID, WHILE, DO, FOR,
		else if (strcmp(buffer[j], "if") == 0)
		{
			InsertElem(&list, TokenCounter, buffer[j], IF, LineNumber, "");
			TokenCounter++;
		}
		else if (strcmp(buffer[j], "else") == 0)
		{
			InsertElem(&list, TokenCounter, buffer[j], ELSE, LineNumber, "");
			TokenCounter++;
		}
		else if (strcmp(buffer[j], "int") == 0)
		{
			InsertElem(&list, TokenCounter, buffer[j], INT, LineNumber, "");
			TokenCounter++;
		}
		else if (strcmp(buffer[j], "return") == 0)
		{
			InsertElem(&list, TokenCounter, buffer[j], RETURN, LineNumber, "");
			TokenCounter++;
		}
		else if (strcmp(buffer[j], "void") == 0)
		{
			InsertElem(&list, TokenCounter, buffer[j], VOID, LineNumber, "");
			TokenCounter++;
		}
		else if (strcmp(buffer[j], "while") == 0)
		{
			InsertElem(&list, TokenCounter, buffer[j], WHILE, LineNumber, "");
			TokenCounter++;
		}
		else if (strcmp(buffer[j], "do") == 0)
		{
			InsertElem(&list, TokenCounter, buffer[j], DO, LineNumber, "");
			TokenCounter++;
		}
		else if (strcmp(buffer[j], "for") == 0)
		{
			InsertElem(&list, TokenCounter, buffer[j], FOR, LineNumber, "");
			TokenCounter++;
		}
		else if (buffer[j][0] < 123 && buffer[j][0] > 64)
		{
			if (buffer[j][0] < 90 || buffer[j][0]>96)
			{
				InsertElem(&list, TokenCounter, buffer[j], ID, LineNumber, "");
				TokenCounter++;
			}
		}
		else
		{
			//	InsertElem(&list, TokenCounter, buffer[j], "");okenCounter++;
		}
	}

	InsertElem(&list, TokenCounter, "ENDOFFILE", END, LineNumber, "");
				TokenCounter++;
}

PTOKENNODE scanner()
{
	printf("scanner start!\n");
	char buffer[400][20] = { "" };//use for whole page of code
	char Buf[300] = { "" };//use for one line 	
	PTOKENNODE theTokenList = NULL;
	CreateListTail(&theTokenList);
	int WordsCounter = 0;
	WordsCounter = GetWordsTobuffer(&Buf, &buffer, WordsCounter);
	InsertIntoLinkList(&buffer, WordsCounter, theTokenList);
	//print_token_list(theTokenList);
	/*for (int i = 0; i < WordsCounter; i++)
	{
		printf("%s\n", buffer[i]);
	}*/
	return theTokenList;
} 