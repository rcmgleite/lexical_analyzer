/*
 * lex.c
 *
 *  Created on: Sep 24, 2015
 *      Author: rafael
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"

/*
 *	Reserved words
 */
#define REZERVED_WORDS_SIZE 7
const char* const reserved_words[] = { "if", "else", "while", "int", "float",
		"return", "const" };

/*
 *	Single operators
 */
#define SINGLE_OPERATORS_SIZE 8
const char* const single_operators[] = {"=", ">", "<", "!", "+", "-", "*", "/" };

/*
 *	Double operators
 */
#define DOUBLE_OPERATORS_SIZE 4
const char* const double_operators[] = {"==", ">=", "<=", "!=" };

/*
 *	delimiters
 */
#define DELIMITERS_SIZE 11
const char delimiters[DELIMITERS_SIZE] = { '{', '}', '[', ']', '(', ')', ',', ';', ' ', '\n', '\t' };

/*
 *	comment begin
 */
const char commentary = '#';

/*
 *	Table that represents the Transductor states
 *
 *
 *			digit	alpha	operator	DELIMITER	COMM_INIT	DOT
 *	INIT
 *	COMMENT
 *	N_INT
 *	FLOAT
 *	ALPHAN
 *	OP
 *	DELIM
 *	LEX_ERROR
 */
const state_t next_state[STATES_SIZE][IN_CLASS_SIZE] = {
		{ ST_NUM_INT,	ST_APLHANUM, 	ST_OPERATOR,	ST_DELIMITER,	ST_COMMENT,	ST_LEX_ERROR },
		{ ST_COMMENT,	ST_COMMENT,	ST_COMMENT,	ST_COMMENT,	ST_COMMENT,	ST_COMMENT },
		{ ST_NUM_INT,	ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END,	ST_NUM_FLOAT },
		{ ST_NUM_FLOAT,	ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END,	ST_LEX_ERROR },
		{ ST_APLHANUM,	ST_APLHANUM,	ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END,	ST_LEX_ERROR },
		{ ST_TOKEN_END,	ST_TOKEN_END,	ST_OPERATOR,	ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END },
		{ ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END,	ST_TOKEN_END },
		{ ST_LEX_ERROR,	ST_LEX_ERROR,	ST_LEX_ERROR,	ST_LEX_ERROR,	ST_LEX_ERROR,	ST_LEX_ERROR }
};

/*
 *	Handlers
 */
void append_input(state_struct_t* param) {
	param->buffer[*(param->buffer_ptr)] = param->curr_input;
	*(param->buffer_ptr)+=1;
	param->buffer[*(param->buffer_ptr)] = 0;

	/*
	 *	Calculate next state;
	 */
	*param->last_state = *param->curr_state;
	*param->curr_state = next_state[*param->curr_state][param->input_class];
}

void handle_delimiter_init(state_struct_t* param) {
	if((param->curr_input == ' ' || param->curr_input == '\t' || param->curr_input == '\n')) {
		*param->curr_state = ST_INIT;
	} else {
		append_input(param);
	}

}

void ignore_input(state_struct_t* param) {
	// just ignore the input
}

void handle_comment_init(state_struct_t* param) {
	*param->curr_state = ST_COMMENT;
}

void handle_comment_delimiter(state_struct_t* param) {
	if(param->curr_input == '\n')
		*param->curr_state = ST_INIT;
}

void handle_token_end(state_struct_t* param) {
	if((param->curr_input == ' ' || param->curr_input == '\t' || param->curr_input == '\n')){
	} else {
		ungetc(param->curr_input, param->fp);
	}
	*param->last_state = *param->curr_state;
	*param->curr_state = ST_TOKEN_END;
}

void handle_error(state_struct_t* param) {
	printf(">>ERROR: BUILD FAILED!\n");
	exit(1);
}

/*
 *			digit	alpha	operator	DELIMITER	COMM_INIT	DOT
 *	INIT
 *	COMMENT
 *	N_INT
 *	FLOAT
 *	ALPHAN
 *	OP
 *	DELIM
 *	LEX_ERROR
 */
void (* state_function[STATES_SIZE][IN_CLASS_SIZE]) (state_struct_t* param) = {
		{ append_input,	append_input,	append_input,	handle_delimiter_init,	handle_comment_init,	handle_error },
		{ ignore_input,	ignore_input,	ignore_input,	handle_comment_delimiter,	ignore_input,	ignore_input },
		{ append_input,	handle_token_end,	handle_token_end,	handle_token_end,	handle_token_end,	append_input },
		{ append_input,	handle_token_end,	handle_token_end,	handle_token_end,	handle_token_end,	handle_error },
		{ append_input,	append_input,	handle_token_end,	handle_token_end,	handle_token_end,	handle_error },
		{ handle_token_end,	handle_token_end,	append_input,	handle_token_end,	handle_token_end,	handle_token_end },
		{ handle_token_end,	handle_token_end,	handle_token_end,	handle_token_end,	handle_token_end,	handle_token_end },
		{ handle_error,	handle_error,	handle_error,	handle_error,	handle_error,	handle_error }
};

/*
 *	Helpers
 */
int is_digit(char c) {
	return c >= '0' && c <= '9';
}

int is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_operator(char c) {
	unsigned i;
	for(i = 0; i < SINGLE_OPERATORS_SIZE; i++) {
		if (c == single_operators[i][0])
			return TRUE;
	}
	return FALSE;
}

int is_delimiter(char c) {
	unsigned i;
	for(i = 0; i < DELIMITERS_SIZE; i++){
		if (c == delimiters[i]) {
			return TRUE;
		}
	}
	return FALSE;
}

int is_comment_begin(char c) {
	return c == commentary;
}

int is_dot(char c) {
	return c == '.';
}

/*
 *	classify input read from file by its type
 *		-> it returns -1 if the type does not match any of the specified by the language
 */
input_class classify_input_class(char c) {
	if(is_digit(c))
		return IN_DIGIT;
	else if(is_alpha(c))
		return IN_APLHA;
	else if(is_operator(c))
		return IN_OPERATOR;
	else if(is_delimiter(c))
		return IN_DELIMITER;
	else if(is_comment_begin(c))
		return IN_COMMENT_BEGIN;
	else if(is_dot(c))
		return IN_DOT;
	else
		return -1;
}

void build_token(state_struct_t* param) {
	unsigned i;
	float f_val;
	int i_val;
	switch(*param->last_state) {
	case ST_APLHANUM:
		for(i = 0; i < REZERVED_WORDS_SIZE; i++) {
			if(strcmp(param->buffer, reserved_words[i]) == 0)
				break;
		}

		if(i == REZERVED_WORDS_SIZE) {
			//TODO TABELA DE SIMBOLOS
			i = -1;
			param->token = new_token(CLASS_IDENTIFIER, &i);
		} else {
			param->token = new_token(CLASS_RESERVED_WORD, &i);
		}
		break;
	case ST_DELIMITER:
		for(i = 0; i < DELIMITERS_SIZE; i++){
			if(param->buffer[0] == delimiters[i])
				break;
		}
		param->token = new_token(CLASS_DELIMITER, &i);
		break;
	case ST_NUM_FLOAT:
		f_val = strtof(param->buffer, NULL);
		param->token = new_token(CLASS_FLOAT, &f_val);
		break;
	case ST_NUM_INT:
		i_val = strtol(param->buffer, NULL, 10);
		param->token = new_token(CLASS_INT, &i_val);
		break;
	case ST_OPERATOR:
		if(*param->buffer_ptr == 1){
			for(i = 0; i < SINGLE_OPERATORS_SIZE; i++) {
				if(param->buffer[0] == single_operators[i][0])
					break;
			}
			param->token = new_token(CLASS_SINGLE_OPERATOR, &i);
		} else {
			for(i = 0; i < DOUBLE_OPERATORS_SIZE; i++) {
				if(param->buffer[0] == double_operators[i][0])
					break;
			}
			param->token = new_token(CLASS_DOUBLE_OPERATOR, &i);
		}
		break;
	default:
		fprintf(stderr, ">> Shouldn't get at 'default' option of build_token\nUnknown error\n");
	}
}

/*
 *	main function
 */
token_t* get_token(FILE *fp) {
	state_struct_t state_struct;
	state_struct.buffer = malloc(BUFFER_SIZE * sizeof(char));
	state_struct.last_state = malloc(sizeof(state_t));
	state_struct.curr_state = malloc(sizeof(state_t));
	state_struct.buffer_ptr = malloc(sizeof(unsigned));
	*state_struct.last_state = ST_INIT;
	*state_struct.curr_state = ST_INIT;
	*state_struct.buffer_ptr = 0;
	state_struct.fp = fp;
	state_struct.token = NULL;

	/*
	 *	Main loop
	 */
	do {
		/*
		 *	Get next input
		 */
		state_struct.curr_input = getc(fp);
		if(state_struct.curr_input == EOF) {
			break;
		}

		/*
		 *	update state_struct
		 */
		state_struct.input_class = classify_input_class(state_struct.curr_input);

		state_function[*state_struct.curr_state][state_struct.input_class](&state_struct);
	} while(*state_struct.curr_state != ST_TOKEN_END);

	/*
	 * 	Build token to be returned
	 */
	if(*state_struct.buffer_ptr != 0){
		printf(">> buffer: %s\n", state_struct.buffer);
		build_token(&state_struct);
	}

	/*
	 *	Free dynamic allocated memory and return
	 */
	free(state_struct.buffer);
	free(state_struct.buffer_ptr);
	free(state_struct.curr_state);
	free(state_struct.last_state);
	return state_struct.token;
}
