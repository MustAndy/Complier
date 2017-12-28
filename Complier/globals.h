/****************************************************/
/* File: globals.h                                  */
/* Global types and vars for C-Minus compiler       */
/* must come before other include files             */
/* Compiler Construction: Principles and Practice   */
/* Book author:  Kenneth C. Louden                  */
/* Programming by Zhiyao Liang                      */
/* MUST FIT 2014 Fall                               */
/****************************************************/


#ifndef _GLOBALS_H_
#define _GLOBALS_H_


#include "util.h"

extern FILE* InputFile; /* source code text file */
extern FILE* listing; /* listing output text file */



extern Boolean G_echoSource; /* to print the source code when reading it. */
extern Boolean G_traceScanner;  /* to print  the token list  obtained by the scanner. */
extern Boolean G_traceParser;  /* to print  the parse tree  obtained by the parser. */
extern Boolean P_debugParser;
extern Boolean P_keepParseTreeOnError;
extern Boolean P_parserExitOnError;
/* When TRUE< print the symbol table that it builds */
extern Boolean G_traceAnalyzer;
extern Boolean S_debugScanner;
extern Boolean S_scannerErrorFound;
extern Boolean A_debugAnalyzer;
extern Boolean G_pause;
extern Boolean G_exitOnError; // Whether to  exit the program when the first error is found

#endif
