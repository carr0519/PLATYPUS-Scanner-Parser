/*	File name:	parser.h
 *	Compiler:	MS Visual Studio 2019
 *	Author:		Alex Carrozzi
 *	Date:		November Decemeber 5, 2019
 *	Purpose:	Provides utilities required by the parser.
 *				This includes: exteral linkage to variables and functions,
 *				preprocessor directives such as include and define constants,
 *				global variables and enumerations, and function declarations.
 *	Functions:	Only declarations	
 */

#pragma once

#include "token.h"
#include "buffer.h"

#define NO_ATTR (-1)	/* attribute for non-enumerated tokens */

/* external linkage to utilities required by the parser */
extern char* kw_table[];
extern int line;
extern pBuffer str_LTBL;
extern Token malar_next_token(void);

static Token lookahead;		/* stores the current Token to be matched by the parser */
int synerrno = 0;			/* a count of the number of syntax errors */

/* symbolic constants for each keyword */
enum keywords {
	ELSE,	  /* = 0 */
	FALSE,	  /* = 1 */
	IF,		  /* = 2 */
	PLATYPUS, /* ... */
	READ,
	REPEAT,
	THEN,
	TRUE,
	WHILE,
	WRITE	  /* = 9 */
};


/* forward declarations */
void match(int pr_token_code, int pr_token_attribute);
void gen_incode(char* str);
void syn_eh(int sync_token_code);
void syn_printe(void);
void parser(void);
void program(void);
void opt_statements(void);
void statements(void);
void statements_p(void);
void statement(void);
void assignment_statement(void);
void selection_statement(void);
void iteration_statement(void);
void assignment_expression(void);
void input_statement(void);
void output_statement(void);
void output_list(void);
void opt_variable_list(void);
void pre_condition(void);
void variable_list(void);
void variable_list_p(void);
void variable_identifier(void);
void relational_operator(void);
void arithmetic_expression(void);
void unary_arithmetic_expression(void);
void additive_arithmetic_expression(void);
void additive_arithmetic_expression_p(void);
void multiplicative_arithmetic_expression(void);
void multiplicative_arithmetic_expression_p(void);
void primary_arithmetic_expression(void);
void string_expression(void);
void string_expression_p(void);
void primary_string_expression(void);
void conditional_expression(void);
void logical_OR_expression(void);
void logical_OR_expression_p(void);
void logical_AND_expression(void);
void logical_AND_expression_p(void);
void relational_expression(void);
void primary_a_relational_expression(void);
void primary_s_relational_expression(void);




