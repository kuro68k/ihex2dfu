#pragma once

typedef enum
{
	ARGTYPE_INT8,
	ARGTYPE_INT16,
	ARGTYPE_INT32,
	ARGTYPE_INT64,
	ARGTYPE_UINT8,
	ARGTYPE_UINT16,
	ARGTYPE_UINT32,
	ARGTYPE_UINT64,
	ARGTYPE_FLOAT,
	ARGTYPE_DOUBLE,
	ARGTYPE_CHAR,
	ARGTYPE_STRING,
	ARGTYPE_BOOL,
} ARGTYPE_t;


/*	Define your arguments here
*/
extern char	*input_filename;
extern char	*output_filename;

#define	ARGUMENT_TABLE \
	{ &input_filename, ARGTYPE_STRING, "hex_file", "input Intel hex file", true }, \
	{ &output_filename, ARGTYPE_STRING, "dfu_file", "output DFU file", false },

//#define	OPTION_TABLE \
//	{ { &float_test, ARGTYPE_FLOAT, "float_opt", "optional float", true }, 'f', "float" }, \
//	{ { &char_test, ARGTYPE_CHAR, "char_opt", "optional char", false }, 'c', "char" },


extern bool cmdargs_parse(int argc, char *argv[]);
extern void cmdargs_print_help(char *app_name);
