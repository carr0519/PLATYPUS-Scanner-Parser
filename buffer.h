#pragma once
/*
	File name: buffer.h
	Compiler: MS Visual Studio 2019
	Author: Alex Carrozzi, 040793835
	Course: CST 8152 – Compilers, Lab Section : 011
	Assignment: 1
	Date: October 2nd, 2019
	Professor: Sv.Ranev
	Purpose: This header file provides the function declarations in the file buffer.c as well as macros for error signalling, bit-wise operations and various buffer values.
			 A function-like macro, b_isfull() is defined using preprocessor conditionals if the including file has defined B_FULL.
			 The buffer structure which the file buffer.c relies heavily on is also defined in this header file with 2 typedefs -- one is a pointer.

	Function list: N/A (no function definitions, only declarations)*/
#ifndef BUFFER_H_
#define BUFFER_H_

/*#pragma warning(1:4001) *//*to enforce C89 type comments  - to make //comments an warning */

#pragma warning(error:4001) /* to enforce C89 comments - to make // comments an error */

/* standard header files */
#include <stdio.h>  /* standard input/output */
#include <malloc.h> /* for dynamic memory allocation*/
#include <limits.h> /* implementation-defined data type ranges and limits */

/* constant definitions */
#define RT_FAIL_1 (-1)         /* operation failure return value 1 */
#define RT_FAIL_2 (-2)         /* operation failure return value 2 */
#define LOAD_FAIL (-2)         /* load fail return value */

#define DEFAULT_INIT_CAPACITY 200   /* default initial buffer capacity */
#define DEFAULT_INC_FACTOR 15       /* default increment factor */

 /*You should add your own constant definitions here */
#define ADDITIVE_MODE 1
#define MULTIPLICATIVE_MODE (-1)
#define FIXED_MODE 0
#define MAX_BUF_CAPACITY (SHRT_MAX-1)

#ifdef B_FULL
#define b_isfull(pBD) ((pBD == NULL) ? (RT_FAIL_1) : (pBD->addc_offset == pBD->capacity))
#endif

/* Add your bit-masks constant definitions here */
#define DEFAULT_FLAGS  0xFFFC
#define SET_EOB 0x0002 /* operand 1 | SET_EOB will preserve operand 1 bits but will set the 2nd LSB to be 1 */
#define RESET_EOB  0xFFFD /* operand 1 & RESET_EOB will preserve operand 1 bits but will set the 2nd LSB to be 0 */
#define CHECK_EOB  0x0002 /* checks the 2nd LSB bit against 1 */
#define SET_R_FLAG 0x0001 /* operand 1 | SET_R_FLAG will preserve operand 1 bits but set the LSB to be 1 */
#define RESET_R_FLAG 0xFFFE /* operand 1 & RESET_R_FLAG will preserve operand 1 bits but will set the LSB to be 0 */
#define CHECK_R_FLAG  0x0001 /* checks the LSB against 1 */

/* user data type declarations */
typedef struct BufferDescriptor {
	char* cb_head;   /* pointer to the beginning of character array (character buffer) */
	short capacity;    /* current dynamic memory size (in bytes) allocated to character buffer */
	short addc_offset;  /* the offset (in chars) to the add-character location */
	short getc_offset;  /* the offset (in chars) to the get-character location */
	short markc_offset; /* the offset (in chars) to the mark location */
	char  inc_factor; /* character array increment factor */
	char  mode;       /* operational mode indicator*/
	unsigned short flags;     /* contains character array reallocation flag and end-of-buffer flag */
} Buffer, * pBuffer;


/* function declarations */
Buffer* b_allocate(short init_capacity, char inc_factor, char o_mode);
pBuffer b_addc(pBuffer const pBD, char symbol);
int b_clear(Buffer* const pBD);
void b_free(Buffer* const pBD);
int (b_isfull)(Buffer* const pBD); /* necessary to wrap b_isfull with parenthesis to avoid name collision with the macro of the same name */
short b_limit(Buffer* const pBD);
short b_capacity(Buffer* const pBD);
short b_mark(pBuffer const pBD, short mark);
int b_mode(Buffer* const pBD);
size_t b_incfactor(Buffer* const pBD);
int b_load(FILE* const fi, Buffer* const pBD);
int b_isempty(Buffer* const pBD);
char b_getc(Buffer* const pBD);
int b_eob(Buffer* const pBD);
int b_print(Buffer* const pBD, char nl);
Buffer* b_compact(Buffer* const pBD, char symbol);
char b_rflag(Buffer* const pBD);
short b_retract(Buffer* const pBD);
short b_reset(Buffer* const pBD);
short b_getcoffset(Buffer* const pBD);
int b_rewind(Buffer* const pBD);
char* b_location(Buffer* const pBD);

#endif


