/*
 *	File name: buffer.c
 *	Compiler: MS Visual Studio 2019
 *	Author: Alex Carrozzi
 *	Date: October 2nd, 2019
 *	Purpose: Provides the utilities to create and manipulate a Buffer. All of the functions are services for a buffer handler
 *			 to be able to do a variety of things such as: adding and retrieving characters from the buffer and printing it's contents.
 *
 *	Function list:  b_allocate()
 *			b_addc()
 *			b_clear()
 *			b_free()
 *			b_isfull()
 *			b_limit()
 *			b_capacity()
 *			b_mark()
 *			b_mode()
 *			b_incfactor()
 *			b_load()
 *			b_isempty()
 *			b_getc()
 *			b_eob()
 *			b_print()
 *			b_compact()
 *			b_rflag()
 *			b_retract()
 *			b_reset()
 *			b_getcoffset()
 *			b_rewind()
 *			b_location()
 */

#include "buffer.h"

/*
 *	Purpose: Dynamically allocates a buffer handler and initializes it's properties
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : calloc(), malloc(), free()
 *	Parameters : init_capacity: short, unit of measurement is in BYTES and should be positive
 *				 inc_factor: char, unit of measurement is in BYTES in ADDITIVE_MODE, percentage in MULTIPLICATIVE_MODE from 1-100 inclusive
 *				 o_mode: char, should be char 'a' or 'm' or 'f' case sensitive
 *	Return value : A valid pointer to a buffer handler or NULL if error
 *	Algorithm : N/A
 */
Buffer* b_allocate(short init_capacity, char inc_factor, char o_mode)
{
	pBuffer pBuf; /* a pointer to the buffer handler, not yet instantiated */

	/* as per the documentation, init_capacity must be between 0 and SHRT_MAX -1 inclusive */
	if (init_capacity < 0 || init_capacity > MAX_BUF_CAPACITY)
		return NULL;

	/* dynamically allocate buffer handler, return NULL if call to calloc() fails (returns NULL)
		ignore warning: assignment within condition expression  -- I've used parenthesis */
	if (!(pBuf = (Buffer*)calloc(1, sizeof(Buffer))))
		return NULL;

	/* dynamically allocate character buffer, free handler and return NULL if call to malloc() fails (returns NULL)
		ignore warning: assignment within condition expression  -- I've used parenthesis */
	if (!(pBuf->cb_head = (char*)malloc(init_capacity == 0 ? DEFAULT_INIT_CAPACITY : init_capacity))) {
		free(pBuf);
		return NULL;
	}

	/* checks parameters for the proper assignment of handler properties: mode, inc_factor and capacity */
	if (init_capacity == 0) {
		switch (o_mode) {
		case 'a': pBuf->mode = ADDITIVE_MODE;
			pBuf->inc_factor = DEFAULT_INC_FACTOR;
			break;

		case 'm': pBuf->mode = MULTIPLICATIVE_MODE;
			pBuf->inc_factor = DEFAULT_INC_FACTOR;
			break;

		case 'f': pBuf->mode = FIXED_MODE;
			pBuf->inc_factor = 0;
			break;

			/* invalid o_mode parameter, free handler and char buffer then return NULL */
		default:  free(pBuf->cb_head);
			free(pBuf);
			return NULL;
		}
	}
	else if (inc_factor == 0 || o_mode == 'f') {
		pBuf->mode = FIXED_MODE;
		pBuf->inc_factor = 0;
	}
	/* no need to check if inc_factor is between 1-255 inclusive. An unsigned char is always between 0-255 and 0 was just checked for */
	else if (o_mode == 'a') {
		pBuf->mode = ADDITIVE_MODE;
		pBuf->inc_factor = inc_factor;
	}
	/* necessary to cast inc_factor to unsigned char as values 128-255 would otherwise be interpreted as negative */
	else if (o_mode == 'm' && (unsigned char)inc_factor <= 100) {
		pBuf->mode = MULTIPLICATIVE_MODE;
		pBuf->inc_factor = inc_factor;
	}
	/* bad parameter set, free buffer handler and char buffer then return NULL */
	else {
		free(pBuf->cb_head);
		free(pBuf);
		return NULL;
	}
	pBuf->capacity = (init_capacity == 0) ? DEFAULT_INIT_CAPACITY : init_capacity;
	pBuf->flags = DEFAULT_FLAGS;
	pBuf->addc_offset = pBuf->getc_offset = pBuf->markc_offset = 0; /* not mentioned the specification for this function but it seems reasonable to include */
	return pBuf;
}


/*
 *	Purpose: Adds the char, symbol, to the buffer if there is space. Attempts to increase the capacity of the buffer if it is full.
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : realloc()
 *	Parameters : pBuffer: A valid pointer to a Buffer structure
 *				 symbol: char, ranging from -128 - 127 inclusive
 *	Return value : A valid pointer to a buffer handler or NULL if error
 *	Algorithm : N/A
 */
pBuffer b_addc(pBuffer const pBD, char symbol)
{
	if (pBD == NULL) return NULL;

	/* realloc_ptr will store the return value of realloc() to avoid a potential dangling pointer
		it is also used to check if the block of memory was moved so we can set the r_flag accordingly */
	void* realloc_ptr;
	short new_capacity; /* if the capacity needs to be increased because the buffer is full, new_capacity is used in the calculations */

	pBD->flags &= RESET_R_FLAG; /* assume no realloction to start */

	/* check if buffer is full */
	if (pBD->addc_offset == pBD->capacity) {

		/* check if capacity already maxed out (SHRT_MAX or SHRT_MAX - 1)*/
		if (pBD->capacity >= MAX_BUF_CAPACITY)
			return NULL;

		/* capacity can be increased, check mode of handler and run the appropriate algorithm */
		switch (pBD->mode) {

		case ADDITIVE_MODE:	new_capacity = pBD->capacity + (unsigned char)pBD->inc_factor;
			if (new_capacity < 0)
				return NULL;
			/* it's possible the new_capacity exceeded MAX_BUF_CAPACITY without overflowing */
			new_capacity = new_capacity == SHRT_MAX ? MAX_BUF_CAPACITY : new_capacity;
			break;

			/* need at least 4 bytes to store the maximum possible value of the multiplication, hence the cast to long*/
		case MULTIPLICATIVE_MODE:  new_capacity = (MAX_BUF_CAPACITY - pBD->capacity) * (long)pBD->inc_factor / 100;
			new_capacity = (new_capacity == 0 || new_capacity + (long)pBD->capacity > MAX_BUF_CAPACITY) ? MAX_BUF_CAPACITY : (new_capacity + pBD->capacity);
			break;

			/* catches FIXED_MODE */
		default:  return NULL;
			break;
		}

		/* assign pointer returned from realloc() to realloc_ptr, if NULL then return NULL */
		if (!(realloc_ptr = realloc(pBD->cb_head, new_capacity)))
			return NULL;

		/* check if the starting address was moved, if so, set R_FLAG and assign cb_head the new starting address */
		if (pBD->cb_head != (char*)realloc_ptr) {
			pBD->flags |= SET_R_FLAG;
			pBD->cb_head = (char*)realloc_ptr;
		}
		pBD->capacity = new_capacity;
	}
	pBD->cb_head[pBD->addc_offset++] = symbol; /* append symbol to the char buffer and increment addc_offset */
	return pBD;
}


/*
 *	Purpose: Resets the add, get and mark char offsets to 0. Leaves the buffer contents alone.
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : 0 on success, -1 if NULL ptr is passed
 *	Algorithm : N/A
 */
int b_clear(Buffer* const pBD)
{
	if (pBD == NULL)  return RT_FAIL_1;

	/* pBD->flags = DEFAULT_FLAGS;  not sure about this */
	return pBD->addc_offset = pBD->getc_offset = pBD->markc_offset = 0; /* asscoiativity of assignment operator is right to left */
}


/*
 *	Purpose: De-allocates the char buffer and the buffer handler from memory
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : free()
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : N/A
 *	Algorithm : N/A
 */
void b_free(Buffer* const pBD)
{
	if (pBD == NULL)  return;
	free(pBD->cb_head); /* free the char buffer before the handler structure, OF COUUUURRSE */
	free(pBD);
}


/*
 *	Purpose: Checks if the buffer handler is full.
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 on error, 0 if not full, 1 if full
 *	Algorithm : N/A
 */
int b_isfull(Buffer* const pBD)
{
	return (pBD == NULL) ? RT_FAIL_1 : (pBD->addc_offset == pBD->capacity);
}

/*
 *	Purpose: Finds the current number of chars added to the buffer.
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 on error, otherwise returns the limit of the buffer
 *	Algorithm : N/A
 */
short b_limit(Buffer* const pBD)
{
	return (pBD == NULL) ? RT_FAIL_1 : pBD->addc_offset;
}


/*
 *	Purpose: Find the size of the buffer measured in bytes
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : 0 on success, -1 if NULL ptr is passed
 *	Algorithm : N/A
 */
short b_capacity(Buffer* const pBD)
{
	return (pBD == NULL) ? RT_FAIL_1 : pBD->capacity;
}


/*
 *	Purpose: Sets the handler property markc_offset to the parameter mark
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 on error, mark otherwise
 *	Algorithm : N/A
 */
short b_mark(pBuffer const pBD, short mark)
{
	/* bound check the mark parameter first, if OK then assign to markc_offset and return it's value */
	return ((pBD == NULL) || (unsigned short)mark > pBD->addc_offset) ? RT_FAIL_1 : (pBD->markc_offset = mark);
}


/*
 *	Purpose: Returns the mode property of the buffer handler
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -2 on error, mode otherwise
 *	Algorithm : N/A
 */
int b_mode(Buffer* const pBD)
{
	/* in case of null ptr, -2 is returned because -1, 0 and 1 are valid modes */
	return (pBD == NULL) ? RT_FAIL_2 : (int)pBD->mode;
}


/*
 *	Purpose: Returns the buffer handler property inc_factor is an unsigned int
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : 0x100 if null ptr, inc_factor otherwise
 *	Algorithm : N/A
 */
size_t b_incfactor(Buffer* const pBD)
{
	return (pBD == NULL) ? (unsigned int)0x100 : (unsigned int)(unsigned char)pBD->inc_factor;
}


/*
 *	Purpose: Populates the char buffer one char at a time by reading from FILE param fi
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : fi: A valid pointer to a FILE struct
 *				 pBD: A valid pointer to a Buffer structure
 *	Return value : NULL if null fi or buffer ptr, -2 if file too big, number of chars added to char buffer otherwise
 *	Algorithm : N/A
 */
int b_load(FILE* const fi, Buffer* const pBD)
{
	/* valid pointer check */
	if (fi == NULL || pBD == NULL)  return RT_FAIL_1;

	char c; /* stores char read from file fi */
	int count = 0; /* # of characters added to buffer successfully */
	for (;;) {
		c = fgetc(fi);
		if (feof(fi))
			return count;

		if (b_addc(pBD, c) == NULL) {
			ungetc(c, fi);
			return LOAD_FAIL;
		}
		++count;
	}
}

/*
 *	Purpose: Tests addc_offset against 0 to check if the buffer is empty
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 if null buffer ptr, 0 if buffer not empty, 1 if buffer is empty
 *	Algorithm : N/A
 */
int b_isempty(Buffer* const pBD)
{
	return (pBD == NULL) ? RT_FAIL_1 : (pBD->addc_offset == 0);
}


/*
 *	Purpose: Returns the char from the buffer indexed with getc_offset
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -2 if null buffer ptr, 0 if can't read more chars from buffer, the char read from the buffer otherwise
 *	Algorithm : N/A
 */
char b_getc(Buffer* const pBD)
{
	if (pBD == NULL)  return RT_FAIL_2;

	/* check if we can read another character from the buffer */
	if (pBD->getc_offset == pBD->addc_offset) {
		pBD->flags |= SET_EOB;
		return 0; 
	}

	pBD->flags &= RESET_EOB;
	return pBD->cb_head[pBD->getc_offset++];
}


/*
 *	Purpose: Checks if the End-Of-Buffer flag bit is set
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 on null buffer ptr, the value of the EOB bit otherwise
 *	Algorithm : N/A
 */
int b_eob(Buffer* const pBD)
{
	return (pBD == NULL) ? RT_FAIL_1 : pBD->flags & CHECK_EOB;
}


/*
 *	Purpose: Prints the buffer contents w/ printf()
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *				 nl: char, signal for printing a newline
 *	Return value : -1 if null buffer ptr, the number of chars printed otherwise
 *	Algorithm : N/A
 */
int b_print(Buffer* const pBD, char nl)
{
	if (pBD == NULL)  return RT_FAIL_1;

	int count; /* # of chars printed to the screen */
	char c; /* stores char read from buffer using b_getc() */
	for (c = b_getc(pBD), count = 0; !b_eob(pBD); c = b_getc(pBD), ++count)
		printf("%c", c);

	if (nl) printf("\n");

	return count;
}


/*
 *	Purpose: Adds the char, symbol, to the buffer and resizes the capacity to be equal to the total number of chars added
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : NULL on error, the same Buffer pointer as the param pBD otherwise
 *	Algorithm : N/A
 */
Buffer* b_compact(Buffer* const pBD, char symbol)
{
	if (pBD == NULL) return NULL;

	/* realloc_ptr will store the return value of realloc() to avoid a potential dangling pointer
	it is also used to check if the block of memory was moved so we can set the r_flag accordingly */
	void* realloc_ptr;

	pBD->flags &= RESET_R_FLAG; /* assume char buffer starting address is not moved to start */

	/* buffer is full and can't be expanded */
	if (pBD->addc_offset == SHRT_MAX)
		return NULL;

	/* realloc() the char buffer to exactly one byte bigger than addc_offset */
	if (!(realloc_ptr = realloc(pBD->cb_head, pBD->addc_offset + 1)))
		return NULL;

	/* did the starting address of the char buffer move? If it did, set the r_flag */
	if (pBD->cb_head != realloc_ptr) {
		pBD->flags |= SET_R_FLAG;
		pBD->cb_head = (char*)realloc_ptr;
	}
	pBD->capacity = pBD->addc_offset + 1;
	pBD->cb_head[pBD->addc_offset++] = symbol; /* append symbol to char buffer and increment addc_offset */
	return pBD;
}


/*
 *	Purpose: Checks if the flag big r_flag is set
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 if null buffer ptr, the value of flag bit r_flag otherwise
 *	Algorithm : N/A
 */
char b_rflag(Buffer* const pBD)
{
	return (pBD == NULL) ? RT_FAIL_1 : pBD->flags & CHECK_R_FLAG;
}

/*
 *	Purpose: Decrements the buffer handler property getc_offset by 1.
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 if null buffer ptr, getc_offset after decrement otherwise
 *	Algorithm : N/A
 */
short b_retract(Buffer* const pBD)
{
	return (pBD == NULL || pBD->getc_offset == 0) ? RT_FAIL_1 : --(pBD->getc_offset);
}


/*
 *	Purpose: Sets the buffer handler property getc_offset to markc_offset
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 if null buffer ptr, getc_offset after assignment
 *	Algorithm : N/A
 */
short b_reset(Buffer* const pBD)
{
	return (pBD == NULL) ? RT_FAIL_1 : (pBD->getc_offset = pBD->markc_offset);
}


/*
 *	Purpose: Returns the value of getc_offset
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 if null buffer ptr, getc_offset otherwise
 *	Algorithm : N/A
 */
short b_getcoffset(Buffer* const pBD)
{
	return (pBD == NULL) ? RT_FAIL_1 : pBD->getc_offset;
}


/*
 *	Purpose: Resets getc_offset and markc_offset to 0
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 if null buffer ptr, getc_offset after reset otherwise
 *	Algorithm : N/A
 */
int b_rewind(Buffer* const pBD)
{
	return (pBD == NULL) ? RT_FAIL_1 : (pBD->getc_offset = pBD->markc_offset = 0); /* associativity is right to left for assignment */
}

/*
 *	Purpose: Returns a pointer to the char buffer indication by markc_offset
 *	Author : Alex Carrozzi
 *	History / Versions: 1.0
 *	Called functions : N/A
 *	Parameters : pBD: A valid pointer to a Buffer structure
 *	Return value : -1 if null buffer ptr, getc_offset after decrement otherwise
 *	Algorithm : N/A
 */ 
char* b_location(Buffer* const pBD)
{
	return (pBD == NULL) ? NULL : (pBD->cb_head + pBD->markc_offset); /* pointer arithmetic, no dereferencing */
}

