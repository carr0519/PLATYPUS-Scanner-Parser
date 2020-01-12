/*	File name:	table.h
 *	Compiler:	MS Visual Studio 2019
 *	Author:		Alex Carrozzi & Andy Ta
 *	Course:		CST 8152 - Compilers
 *	Lab Secion: 011
 *	Assignment:	2
 *	Date:		November 2, 2019
 *	Professor:	Sv Ranev
 *	Purpose:	Defines the Deterministic Finite Automata for the PLATYPUS language.
 *				The DFA is defined the following tables: transition table, the accepting state 
 *				of each state, pointers to accepting state, as well as the declarations 
 *				of the those pointers. An addition keyword lookup table is defined here.
 *	Functions:	The following are all declarations:
 *				aa_func02()
 *				aa_func03()
 *				aa_func08()
 *				aa_func05()
 *				aa_func10()
 *				aa_func11()
 */

#ifndef  TABLE_H_
#define  TABLE_H_ 

#ifndef BUFFER_H_
#include "buffer.h"
#endif

#ifndef NULL
#include <_null.h> /* NULL pointer constant is defined there */
#endif

#define KWT_SIZE 10		/* keywords in the PLATYPUS language  */
#define ALOC_BUF_FAIL 1	/* call to b_allocate() failed (returned NULL) */
#define STR_BUF_FULL 2	/* string literal buffer full -- can't add the lexeme */
#define SEOF 255		/* Source end-of-filesentinel symbol */
#define SEOB 0			/* Source end-of-buffer symbolic constant */
#define ES 11			/* Error state  with no retract */
#define ER 12			/* Error state  with retract */
#define IS -1			/* Invalid state */

#define TABLE_COLUMNS 8		/* transition table column count */
 /*	Column Headers

	Index 0:	[a-zA-Z]
	Index 1:	0
	Index 2:	[1-9]
	Index 3:	.
	Index 4:	@
	Index 5:	"
	Index 6:	SEOF
	Index 7:	other
*/
/* transition table - type of states defined in separate table */
int  st_table[][TABLE_COLUMNS] = {
	/* State 0  */  {1, 6, 4, ES, ES, 9, ES, ES},
	/* State 1  */  {1, 1, 1, 2, 3, 2, 2, 2},
	/* State 2  */  {IS, IS, IS, IS, IS, IS, IS, IS},
	/* State 3  */  {IS, IS, IS, IS, IS, IS, IS, IS},
	/* State 4  */  {ES, 4, 4, 7, 5, 5, 5, 5},
	/* State 5  */  {IS, IS, IS, IS, IS, IS, IS, IS},
	/* State 6  */  {ES, 6, ES, 7, ES, 5, 5, 5},
	/* State 7  */  {8, 7, 7, 8, 8, 8, 8, 8},
	/* State 8  */  {IS, IS, IS, IS, IS, IS, IS, IS},
	/* State 9  */  {9, 9, 9, 9, 9, 10, ER, 9},
	/* State 10 */  {IS, IS, IS, IS, IS, IS, IS, IS},
	/* State 11 */  {IS, IS, IS, IS, IS, IS, IS, IS},
	/* State 12 */  {IS, IS, IS, IS, IS, IS, IS, IS},
};


/* Accepting state table definition */
#define ASWR 1		/* accepting state with retract */
#define ASNR 2		/* accepting state with no retract */
#define NOAS 0		/* not accepting state */

int as_table[] = 
{	
	NOAS,	/* State 0  */
	NOAS,	/* State 1  */
	ASWR,	/* State 2  */
	ASNR,	/* State 3  */
	NOAS,	/* State 4  */
	ASWR,	/* State 5  */
	NOAS,	/* State 6  */
	NOAS,	/* State 7  */
	ASWR,	/* State 8  */
	NOAS,	/* State 9  */
	ASNR,	/* State 10 */
	ASNR,	/* State 11 */
	ASWR	/* State 12 */
};


/* Accepting State Function Prototypes */
Token aa_func02(char* lexeme);
Token aa_func03(char* lexeme);
Token aa_func08(char* lexeme);
Token aa_func05(char* lexeme);
Token aa_func10(char* lexeme);
Token aa_func11(char* lexeme);


/* Defining a new type: pointer to function (of one char * argument) returning Token */
typedef Token (*PTR_AAF)(char* lexeme);


/* Accepting function (action) callback table (array) definition */
PTR_AAF aa_table[] = 
{ 
	NULL,
	NULL,
	aa_func02,
	aa_func03,
	NULL,
	aa_func05,
	NULL, 
	NULL,
	aa_func08,
	NULL,
	aa_func10,
	aa_func11,
	aa_func11
};


/* Keyword lookup table */
char* kw_table[] =
{
	"ELSE",
	"FALSE",
	"IF",
	"PLATYPUS",
	"READ",
	"REPEAT",
	"THEN",
	"TRUE",
	"WHILE",
	"WRITE"
};

#endif

