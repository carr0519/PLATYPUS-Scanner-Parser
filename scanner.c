/*	File name:	scanner.c
 *	Compiler:	MS Visual Studio 2019
 *	Author:		Alex Carrozzi
 *	Professor:	Sv Ranev
 *	Purpose:	Implements a token-driven and DFA-driven scanner hybrid which is used
 *				to generate Tokens as defined by the PLATYPUS language specification document.
 *	Functions:	scanner_init()
 *			malar_next_token()
 *			get_next_state()
 *			char_class()
 *			aa_func02()
 *			aa_func03()
 *			aa_func08()
 *			aa_func05()
 *			aa_func10()
 *			aa_func11()
 *			iskeyword()			   
 */


 /* The #define _CRT_SECURE_NO_WARNINGS should be used in MS Visual Studio projects
  * to suppress the warnings about using "unsafe" functions like fopen()
  * and standard sting library functions defined in string.h.
  * The define does not have any effect in Borland compiler projects.
  */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>   /* standard input / output */
#include <ctype.h>   /* conversion functions */
#include <stdlib.h>  /* standard library functions and constants */
#include <string.h>  /* string functions */
#include <limits.h>  /* integer types constants */
#include <float.h>   /* floating-point types constants */

/*	#define NDEBUG        to suppress assert() call */
#include <assert.h>  /* assert() prototype */

/* project header files */
#include "buffer.h"
#include "token.h"
#include "table.h"

#define DEBUG  /* for conditional processing */
#undef  DEBUG

/*	Global objects - variables */
/*	This buffer is used as a repository for string literals.
	It is defined in platy_st.c */
extern pBuffer str_LTBL;	/* String literal table */
int line;					/* current line number of the source code */
extern int scerrnum;		/* defined in platy_st.c - run-time error number */


/* Local(file) global objects - variables */
static pBuffer lex_buf;		/*pointer to temporary lexeme buffer*/
static pBuffer sc_buf;		/*pointer to input source buffer*/
/* No other global variable declarations/definitiond are allowed */


/* scanner.c static(local) function  prototypes */
static int char_class(char c);	/* character class function */
static int get_next_state(int, char);	/* state machine function	 */
static int iskeyword(char* kw_lexeme);	/* keywords lookup functuion */
Token aa_func02(char* lexeme);	/* accepting state: AVID/ KW */
Token aa_func03(char* lexeme);	/* accepting state: SVID	 */
Token aa_func08(char* lexeme);	/* accepting state:	FPL		 */
Token aa_func05(char* lexeme);	/* accepting state: IL		 */
Token aa_func10(char* lexeme);	/* accepting state: SL		 */
Token aa_func11(char* lexeme);	/* accepting state: ES		 */

/* non-static function prototypes */
int scanner_init(pBuffer psc_buf);
Token malar_next_token(void);



/*Initializes scanner */
int scanner_init(pBuffer psc_buf) {
	if (b_isempty(psc_buf)) return EXIT_FAILURE;	/*1*/
	/* in case the buffer has been read previously */
	b_rewind(psc_buf);
	b_clear(str_LTBL);
	line = 1;
	sc_buf = psc_buf;
	return EXIT_SUCCESS;	/* 0 */
/*   scerrnum = 0; */		/* no need - global ANSI C */
}


/*	Purpose:	Reads from the input buffer one char at a time and returns a Token
 *				structure once it finds a token pattern which matches a lexeme found
 *				in the stream of input symbols.
 *	Author:		Alex Carrozzi
 *	History / Versions:	1.0
 *	Called functions:	b_getc(), b_retract(), b_eob(), sprintf(), b_mark(), get_next_state(), 
 *						b_getcoffset(), b_allocate(), strcpy(), b_addc(), b_free()
 *	Parameters:		None
 *	Return value:	A Token structure with a code identifying the type of token and sometimes an attribute
 *					which stores the value associated with the token.
 *	Algorithm:	First checks if the char is a simple token consisting of just one or a few chars defined
 *				in the grammar of PLATYPUS. If not, then the DFA will begin creating a lexeme and try to 
 *				generate one of the following more complex tokens: Keyword, AVID, SVID, IL, FPL, SL, 
 *				or an error if the lexeme doesn't match any valid pattern.
 */
Token malar_next_token(void)
{
	Token t = { 0 };	/* token to return after pattern recognition. Set all structure members to 0 */
	unsigned char c;	/* input symbol */
	int state = 0;		/* initial state of the FSM */
	short lexstart;		/* start offset of a lexeme in the input char buffer (array) */
	short lexend;		/* end offset of a lexeme in the input char buffer (array) */

	/* endless loop broken by token returns it will generate a warning */
	while (1) {

		c = b_getc(sc_buf);		/* read the next char from the input buffer */

		/* begin token driven scanner */
		switch (c) {

		/* line terminator check: increment line count and start the next iteration */
		case '\r': case '\n':
			if (c == '\r' && (c = b_getc(sc_buf)) != '\n') /* if \r\n, consume the \n char as well to avoid double counting newline */
				b_retract(sc_buf);						   /* if not, retract the other char */
			++line;
			continue;

		/* ignore leading white space check and start the next iteration */
		case ' ': case '\t': case '\v': case '\f':
			continue;

		case '=': /* check the next char for another '=' -- possible equality operator token */
			if ((c = b_getc(sc_buf)) == '=') {
				t.code = REL_OP_T;
				t.attribute.rel_op = EQ;
				return t;
			}
			/* put other char back in the buffer and return assignment operator token */
			b_retract(sc_buf);
			t.code = ASS_OP_T;
			return t;

		case '<': /* check next char for possible '<>' not equal operator or '<<' concatenation operator token */
			if ((c = b_getc(sc_buf)) == '>') {
				t.code = REL_OP_T;
				t.attribute.rel_op = NE;
				return t;
			}
			if (c == '<') {
				t.code = SCC_OP_T;
				return t;
			}
			/* put other char back in the buffer and return less than relational operator token */
			b_retract(sc_buf);
			t.code = REL_OP_T;
			t.attribute.rel_op = LT;
			return t;

		case '!': /* check next token for possible inline comment -- if not, produce an error token, either way ignore the rest of the line */
			if ((c = b_getc(sc_buf)) == '!') {
				/* ignore the rest of the line and start next iteration */
				while ((c = b_getc(sc_buf)) != '\r' && c != '\n' && c != SEOF && c != SEOB)
					;
				/* end-of-file indicator on this line? (don't retract if sc_buf is at the end of the buffer (SEOB)) */
				if (c == SEOF || c == SEOB)
					b_retract(sc_buf);
				else if (c == '\r' || c == '\n') {
					++line;
					if (c == '\r' && b_getc(sc_buf) != '\n')		/* if '\r\n' consume the \n as well to avoid double counting newline */
						b_retract(sc_buf);
				}
				continue;
			}
			/* if the next char is not '!' assume a comment was intended so ignore the rest of the line anyways but return an error token */
			t.code = ERR_T;
			sprintf(t.attribute.err_lex, "!%c", c);
			/* ignore the rest of the line and return error token */
			while ((c = b_getc(sc_buf)) != '\r' && c != '\n' && c != SEOF && c != SEOB)
				;
			/* end-of-file indicator on this line? (don't retract if sc_buf is at the end of the buffer (SEOB)) */
			if (c == SEOF || c == SEOB)
				b_retract(sc_buf);
			else if (c == '\r' || c == '\n') {
				++line;
				if (c == '\r' && b_getc(sc_buf) != '\n')		/* if '\r\n' consume the \n as well to avoid double counting newline */
					b_retract(sc_buf);
			}
			return t;

		case '.':	/* check for logical .AND. & .OR. */
			/* set mark at getc_offset so we can reset back to it, if necessary */
			b_mark(sc_buf, b_getcoffset(sc_buf));	
			if (b_getc(sc_buf) == 'A' && b_getc(sc_buf) == 'N' && b_getc(sc_buf) == 'D' && b_getc(sc_buf) == '.') {
				t.code = LOG_OP_T;
				t.attribute.log_op = AND;
				return t;
			}
			b_reset(sc_buf);
			if (b_getc(sc_buf) == 'O' && b_getc(sc_buf) == 'R' && b_getc(sc_buf) == '.') {
				t.code = LOG_OP_T;
				t.attribute.log_op = OR;
				return t;
			}
			b_reset(sc_buf);
			/* if not a logical operator than simply produce an error token */
			t.code = ERR_T;
			strcpy(t.attribute.err_lex, ".");
			return t;

		/* the rest of the symbols have no ambiguity as to the token they can produce */
		case '>':
			t.code = REL_OP_T;
			t.attribute.rel_op = GT;
			return t;

		case '(':
			t.code = LPR_T;
			return t;

		case ')':
			t.code = RPR_T;
			return t;

		case '{':
			t.code = LBR_T;
			return t;

		case '}':
			t.code = RBR_T;
			return t;

		case ';':
			t.code = EOS_T;
			return t;

		case ',':
			t.code = COM_T;
			return t;

		case '+':
			t.code = ART_OP_T;
			t.attribute.arr_op = PLUS;
			return t;

		case '-':
			t.code = ART_OP_T;
			t.attribute.arr_op = MINUS;
			return t;

		case '*':
			t.code = ART_OP_T;
			t.attribute.arr_op = MULT;
			return t;

		case '/':
			t.code = ART_OP_T;
			t.attribute.arr_op = DIV;
			return t;

		/* source end-of-file/buffer symbols */
		case SEOF: case SEOB:
			t.code = SEOF_T;
			t.attribute.seof = (c == SEOF) ? SEOF_EOF : SEOF_0;
			return t;
		}


		/* end token driven scanner */

		/*************************************************************************************/

		/* begin transition-table driven DFA */


		/* set the mark to the current value of getcoffset (-1 to compensate because the offset looks forward)
			this is also start of the lexeme */
		lexstart = b_mark(sc_buf, b_getcoffset(sc_buf) - 1);

		/* get char from buffer -> change states based on current state and char -> repeat until at an accepting state */
		for (state = get_next_state(state, c); as_table[state] == NOAS; state = get_next_state(state, (char)c))
			if ((c = b_getc(sc_buf)) == '\r' || c == '\n') {
				++line;
				/* if '\r\n' consume the \n as well to avoid double counting newline */
				if (c == '\r' && b_getc(sc_buf) != '\n')
					b_retract(sc_buf);
			}

		/* retract getc_offset if accepting state allows it */
		if (as_table[state] == ASWR)
			b_retract(sc_buf);

		/* lexend is the value of getc_offset once at an accepting state */
		lexend = b_getcoffset(sc_buf);

		/*  temporary buffer for writing the stream of symbols to. capacity is fixed and 
			equal to the difference between lexend and lexstart + 1 for the null byte '\0' */
		if ((lex_buf = b_allocate((lexend - lexstart) + 1, 0, 'f')) == NULL) {
			scerrnum = ALOC_BUF_FAIL;
			t.code = RTE_T;
			strcpy(t.attribute.err_lex, "RUN TIME ERROR: ");
			return t;
		}

		/* set getc_offset back to the first symbol */
		while (b_retract(sc_buf) != lexstart)
			;

		/* write the lexeme to the lexeme buffer */
		while (b_getcoffset(sc_buf) != lexend)
			b_addc(lex_buf, b_getc(sc_buf));

		b_addc(lex_buf, '\0');		/* terminate the lexeme with a null char */

		/* fetch and then dereference the appropriate pointer to accepting state
		   function and pass in the lexeme buffer which has the stream of tokens */
		t = (aa_table[state])(lex_buf->cb_head);
		b_free(lex_buf);	/* credit to Prof Ranev for this statement */
		return t;			/* return the Token */
	}
}


/*
 *	Purpose:	Determines the label (int) of the next state according to the DFA.
 *	History / Versions:	1.0
 *	Called functions:	char_class(), assert(), printf(), exit()
 *	Parameters:		state: int, the current state of the lexeme.
 *					c: char, the most recently read symbol from the input buffer.
 *	Return value:	int, the label (int) of the next state.
 *					if it's an illegal state (IS) the program is aborted with a call to exit().
 *	Algorithm:	N/A
 */
int get_next_state(int state, char c)
{
	int col;		/* the column index in the TT */
	int next;		/* the state to transition to next */
	col = char_class(c);			/* which column in the TT does the symbol fall under? */
	next = st_table[state][col];	/* index the symbol table to get to the next state */

#ifdef DEBUG
	printf("Input symbol: %c Row: %d Column: %d Next: %d \n", c, state, col, next);
#endif

	assert(next != IS);

#ifdef DEBUG
	if (next == IS) {
		printf("Scanner Error: Illegal state:\n");
		printf("Input symbol: %c Row: %d Column: %d\n", c, state, col);
		exit(1);
	}
#endif
	return next;	/* return token */
}


/*
 *	Purpose:	Determines the column position in the Transition Table that the char, c, falls under.
 *	Author:		Alex Carrozzi
 *	History / Versions:	1.0
 *	Called functions:	None
 *	Parameters:		c: char, the most recently read char from the input buffer.
 *	Return value:	the column position (index) of the Transition Table.
 *	Algorithm:	N/A
 */
int char_class(char c)
{
	if (isalpha(c))	return 0;
	if (c == '0')	return 1;
	if (isdigit(c))	return 2;
	if (c == '.')	return 3;
	if (c == '@')	return 4; 
	if (c == '"')	return 5;
	if (c == EOF || c == SEOB) return 6;
	return 7;	/* other */
}


/*
 *	Purpose:	Accepting state function for arithmetic variable identifers and keywords (AVID / KW).
 *	Author:		Alex Carrozzi
 *	History / Versions:	1.0
 *	Called functions:	iskeyword(), strncpy(), strlen()
 *	Parameters:		lexeme: char*, pointer to the starting address of the lexeme.
 *	Return value:	A Token struct with it's code and attribute set based on the nature of the lexeme.
 *	Algorithm:	N/A
 */
Token aa_func02(char* lexeme)
{
	Token t = { 0 };	/* token to return after pattern recognition. Set all structure members to 0 */
	int kw_idx;			/* keyword index, -1 if no match */

	/* check if lexeme is a keyword first */
	if ((kw_idx = iskeyword(lexeme)) >= 0) {
		t.code = KW_T;
		t.attribute.kwt_idx = kw_idx;
		return t;
	}
	/* generate AVID token and store the lexeme in the attribute */
	t.code = AVID_T;
	strncpy(t.attribute.vid_lex, lexeme, VID_LEN); 
	t.attribute.vid_lex[strlen(lexeme) > VID_LEN ? VID_LEN : strlen(lexeme)] = '\0';	/* null terminate the string */
	return t;
}


/*
 *	Purpose:	Accepting state function for string variable identifers (SVID).
 *	Author:		Alex Carrozzi
 *	History / Versions:	1.0
 *	Called functions:	strlen(), strncpy(), strcpy()
 *	Parameters:		lexeme: char*, pointer to the starting address of the lexeme.
 *	Return value:	A Token struct with it's code and attribute set based on the nature of the lexeme.
 *	Algorithm:	N/A
 */
Token aa_func03(char* lexeme)
{
	Token t = { 0 };		/* token to return after pattern recognition. Set all structure members to 0 */
	int len = strlen(lexeme);	/* lexeme length */
	t.code = SVID_T;	

	/* safely copy lexeme to token attribute appended with an @ symbol */
	if (len > VID_LEN) {
		strncpy(t.attribute.vid_lex, lexeme, VID_LEN - 1);		/* copy VID_LEN - 1 chars to attribute */
		strcpy(t.attribute.vid_lex + (VID_LEN - 1), "@\0");		/* append "@\0" */
	}
	else {
		strcpy(t.attribute.vid_lex, lexeme);	/* copy lexeme (no concern of buffer overflow) */
		t.attribute.vid_lex[len] = '\0';		/* null terminate the string */
	}
	return t; /* return the Token */
}


/*
 *	Purpose:	Accepting state function for floating-point literal (FPL).
 *	Author:		Alex Carrozzi
 *	History / Versions:	1.0
 *	Called functions:	strtod(), aa_func11()
 *	Parameters:		lexeme: char*, pointer to the starting address of the lexeme.
 *	Return value:	A Token struct with it's fields set based on the nature of the lexeme.
 *	Algorithm:	N/A
 */
Token aa_func08(char* lexeme)
{
	Token t = { 0 };		/* token to return after pattern recognition. Set all structure members to 0 */
	double dbl = strtod(lexeme, NULL);		/* double representation of the lexeme */

	/* generate error token if lexeme fails boundary check */
	if (dbl != 0.0 && (dbl > FLT_MAX || dbl < FLT_MIN))
		return (aa_table[ES])(lexeme);		/* call error accepting state function */

	/* generate floating-point token */
	t.code = FPL_T;
	t.attribute.flt_value = (float)dbl;
	return t;
}


/*
 *	Purpose:	Accepting state function for integer literal (IL).
 *	Author:		Alex Carrozzi
 *	History / Versions:	1.0
 *	Called functions:	strlen(), atol(), aa_func11()
 *	Parameters:		lexeme: char*, pointer to the starting address of the lexeme.
 *	Return value:	A Token struct with it's fields set based on the nature of the lexeme.
 *	Algorithm:	N/A
 */
Token aa_func05(char* lexeme)
{
	Token t = { 0 };	/* token to return after pattern recognition. Set all structure members to 0 */
	int len = strlen(lexeme);		/* lexeme length */
	long lng_lexeme = atol(lexeme); /* long representation of lexeme */

	/* generate error token if lexeme fails boundary check */
	if (len > INL_LEN || lng_lexeme > SHRT_MAX)
		return (aa_table[ES])(lexeme);		/* call error accepting state function */

	/* generate integer literal token */
	t.code = INL_T;
	t.attribute.int_value = (int)lng_lexeme;
	return t;
}


/*
 *	Purpose:	Accepting state function for string literal (SL).
 *	Author:		Alex Carrozzi
 *	History / Versions:	1.0
 *	Called functions:	b_limit(), strlen(), b_addc()
 *	Parameters:		lexeme: char*, pointer to the starting address of the lexeme.
 *	Return value:	A Token struct with it's fields set based on the nature of the lexeme.
 *	Algorithm:	N/A
 */
Token aa_func10(char* lexeme)
{
	Token t = { 0 };	/* token to return after pattern recognition. Set all structure members to 0 */
	t.code = STR_T;

	/* Token attribute int_value is the next availble position to write to in the string literal buffer. 
		This value is addc_offset which can be retreived with a call to b_limit() */
	t.attribute.int_value = b_limit(str_LTBL);

	lexeme[strlen(lexeme)-1] = '\0';	/* overwrite closing quotation mark with null char */

	/* the quoation mark at the beginning of the lexeme should be ignored so the lexeme is incremented once before looping */
	for (++lexeme; *lexeme; b_addc(str_LTBL, *lexeme++))
		;

	/* full buffer is checked only once, here when the last char is added */
	if (b_addc(str_LTBL, '\0') == NULL) {
		scerrnum = STR_BUF_FULL;
		t.code = RTE_T;
		strcpy(t.attribute.err_lex, "RUN TIME ERROR: ");
		return t;
	}

	/* terminate the string and return the Token */	
	return t;
}


/*
 *	Purpose:	Accepting state for error state -- generates an error token.
 *	Author:		Alex Carrozzi
 *	History / Versions:	1.0
 *	Called functions:	strlen(), strncpy(), strcpy()
 *	Parameters:		lexeme: char*, pointer to the starting address of the lexeme.
 *	Return value:	A Token struct with it's fields set based on the nature of the lexeme.
 *	Algorithm:	N/A
 */
Token aa_func11(char* lexeme)
{
	Token t = { 0 };	/* token to return after pattern recognition. Set all structure members to 0 */
	t.code = ERR_T;		/* set token code to error value */
	int len = strlen(lexeme);	 /* lexeme length */
	
	/* copy lexeme into error token attribute safely */
	if (len > ERR_LEN) {
		strncpy(t.attribute.err_lex, lexeme, ERR_LEN-3);		/* copy lexeme to token attribute, leave space for ellipses */
		strcpy(t.attribute.err_lex + (ERR_LEN-3), "...\0");		/* append ellipses and null terminator */
	}
	else {
		strcpy(t.attribute.err_lex, lexeme);	/* copy lexeme to token attribute (no concern of buffer overflow) */
		t.attribute.err_lex[len] = '\0';		/* append null terminator */
	}
	return t;	/* return the Token */
}


/*
 *	Purpose:	Determines whether the scanner has found a keyword as defined in the array kw_lookup[].
 *	Author:		Alex Carrozzi
 *	History / Versions:	1.0
 *	Called functions:	strcmp()
 *	Parameters:		lexeme:	char*, pointer to the starting address of the lexeme.
 *	Return value:	the index of the matching keyword in the kw_lookup array.
 *					-1 if no keyword matches the lexeme.
 *	Algorithm:	Simply loops through the array of keywords and compares the string against the lexeme
				and returns the index if an identical match is found.
 */
int iskeyword(char* kw_lexeme)
{
	int index;		/* index of keyword in kw_table, also used as loop control */
	for (index = 0; index < KWT_SIZE; ++index)
		if (!strcmp(kw_lexeme, kw_table[index]))	 /* strcmp() will return 0 if an exact match is found */
			return index;

	return -1;		/* no match */
}




