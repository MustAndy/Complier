#include"util.h"
#include"globals.h"
#include "libs.h"


 const char * string_clone(const char * s, int lineno)
{ 
	int n;
	char * t;

	if (s==NULL) return NULL;

	n = strlen(s)+1;
	t = (char *)malloc(n*sizeof(char)); 

	if (t==NULL)
		fprintf(listing,"Out of memory error at line %d\n", lineno);
	else strcpy(t,s);

	return t;
}