/*	Handle command line arguments
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "cmdargs.h"

typedef struct
{
	void		*target;
	ARGTYPE_t	type;
	char		*short_description;
	char		*help;
	bool		required;
} CMDARGUMENT_t;

typedef struct
{
	CMDARGUMENT_t	arg;
	char	short_opt;		// single character option name
	char	*long_opt;		// string option name
} CMDOPTION_t;


const CMDARGUMENT_t arg_list[] = {
	ARGUMENT_TABLE
};
#define	NUM_ARGS	(sizeof(arg_list) / sizeof(arg_list[0]))

#ifdef OPTION_TABLE
const CMDOPTION_t opt_list[] = {
	OPTION_TABLE
};
#define	NUM_OPTS	(sizeof(opt_list) / sizeof(opt_list[0]))
#endif


bool parse(char *arg, uint8_t type, void *target)
{
	uint64_t vu64 = 0;
	int64_t v64 = 0;
	double vd = 0;
	switch (type)
	{
	case ARGTYPE_INT8:
	case ARGTYPE_INT16:
	case ARGTYPE_INT32:
	case ARGTYPE_INT64:
		errno = 0;
		v64 = strtoll(arg, NULL, 0);
		if (errno != 0)
			return false;
		break;

	case ARGTYPE_UINT8:
	case ARGTYPE_UINT16:
	case ARGTYPE_UINT32:
	case ARGTYPE_UINT64:
		errno = 0;
		vu64 = strtoull(arg, NULL, 0);
		if (errno != 0)
			return false;
		break;

	case ARGTYPE_FLOAT:
	case ARGTYPE_DOUBLE:
		errno = 0;
		vd = strtod(arg, NULL);
		if (errno != 0)
			return false;
		break;
	}

	switch (type)
	{
	case ARGTYPE_INT8:
		if ((v64 < INT8_MIN) || (v64 > INT8_MAX))
			return false;
		*(int8_t *)target = (int8_t)v64;
		break;

	case ARGTYPE_INT16:
		if ((v64 < INT16_MIN) || (v64 > INT16_MAX))
			return false;
		*(int16_t *)target = (int16_t)v64;
		break;

	case ARGTYPE_INT32:
		if ((v64 < INT32_MIN) || (v64 > INT32_MAX))
			return false;
		*(int32_t *)target = (int32_t)v64;
		break;

	case ARGTYPE_INT64:
		*(int64_t *)target = v64;
		break;

	case ARGTYPE_UINT8:
		if (vu64 > UINT8_MAX)
			return false;
		*(uint8_t *)target = (uint8_t)vu64;
		break;

	case ARGTYPE_UINT16:
		if (vu64 > UINT16_MAX)
			return false;
		*(uint16_t *)target = (uint16_t)vu64;
		break;

	case ARGTYPE_UINT32:
		if (vu64 > UINT32_MAX)
			return false;
		*(uint32_t *)target = (uint32_t)vu64;
		break;

	case ARGTYPE_UINT64:
		*(uint64_t *)target = vu64;
		break;

	case ARGTYPE_FLOAT:
		*(float *)target = (float)vd;
		break;

	case ARGTYPE_DOUBLE:
		*(double *)target = vd;
		break;

	case ARGTYPE_CHAR:
		if (strlen(arg) > 1)
		{
			printf("Argument \"%s\" too long (max 1 character).\n", arg);
			return false;
		}
		*(char *)target = arg[0];
		break;

	case ARGTYPE_STRING:
		*(char **)target = arg;
		break;

	default:
		return false;
	}

	return true;
}

bool cmdargs_parse(int argc, char *argv[])
{
	bool consumed[256] = { false };
	bool found_args[NUM_ARGS] = { false };
#ifdef OPTION_TABLE
	bool found_opts[NUM_OPTS] = { false };
#endif

	int i, count;
	//printf("argc: %d\n", argc);


	// options
#ifdef OPTION_TABLE
	count = 0;
	for (int i = 1; i < argc; i++)
	{
		if ((argv[i][0] != '-' && argv[i][0] != '/'))
			continue;
		//printf("opt: \"%s\"\n", argv[i]);

		size_t len = strlen(argv[i]);
		if (len == 1)
		{
			printf("Option missing from \"%s\".\n", argv[i]);
			return false;
		}

		// find option
		bool match = false;
		for (count = 0; count < NUM_OPTS; count++)
		{
			if (len > 2)
			{
				if (strcmp(&argv[i][1], opt_list[count].long_opt) == 0)
					match = true;
			}
			else
			{
				if (argv[i][1] == opt_list[count].short_opt)
					match = true;
			}
			if (match)
				break;
		}
		if (!match)
		{
			printf("Unknown option \"%s\".\n", argv[i]);
			return false;
		}

		if (found_opts[count])	// already seen this option
		{
			printf("Duplicate option \"%s\".\n", argv[i]);
			return false;
		}
		found_opts[count] = true;

		// parse option
		consumed[i] = true;
		if (opt_list[count].arg.type == ARGTYPE_BOOL)
			*(bool *)opt_list[count].arg.target = true;
		else
		{
			if (i >= (argc - 1))	// ran out of arguments
			{
				printf("Missing argument for \"%s\".\n", argv[i]);
				return false;
			}
			i++;
			consumed[i] = true;

			if (!parse(argv[i], opt_list[count].arg.type, opt_list[count].arg.target))
			{
				printf("Unable to parse \"%s\".\n", argv[i]);
				return false;
			}
		}
	}
#endif

	// other arguments
	count = 0;
	for (int i = 1; i < argc; i++)
	{
		if (consumed[i])		// already consumed
			continue;
		//printf("arg: \"%s\"\n", argv[i]);

		if (!parse(argv[i], arg_list[count].type, arg_list[count].target))
		{
			printf("Unable to parse \"%s\".\n", argv[i]);
			return false;
		}

		found_args[count] = true;
		count++;
	}

	// check if all required arguments were found
	for (i = 0; i < NUM_ARGS; i++)
	{
		if (arg_list[i].required && !found_args[i])
		{
			printf("Argument \"%s\" is required.\n", arg_list[i].short_description);
			return false;
		}
	}
#ifdef OPTION_TABLE
	for (i = 0; i < NUM_OPTS; i++)
	{
		if (opt_list[i].arg.required && !found_opts[i])
		{
			printf("Argument \"%s\" is required.\n", opt_list[i].arg.short_description);
			return false;
		}
	}
#endif

	return true;
}

void cmdargs_print_help(char *app_name)
{
	printf("%s", app_name);

#ifdef OPTION_TABLE
	printf(" [options]");
#endif

	for (int i = 0; i < NUM_ARGS; i++)
		printf(arg_list[i].required ? " %s" : " [%s]", arg_list[i].short_description);
	putchar('\n');

#ifdef OPTION_TABLE
	printf("Options:\n");
	for (int i = 0; i < NUM_OPTS; i++)
	{
		// option and optional parameter
		char buffer[79];
		if (opt_list[i].arg.type == ARGTYPE_BOOL)
		{
			snprintf(buffer, sizeof(buffer), "  -%c %s",
				opt_list[i].short_opt, opt_list[i].long_opt);
		}
		else
		{
			snprintf(buffer, sizeof(buffer), "  -%c %s <%s>",
				opt_list[i].short_opt, opt_list[i].long_opt,
				opt_list[i].arg.short_description);
		}
		printf("%s", buffer);

		// description
		const int desc_tab = 32;
		int len = strlen(buffer);
		if (len >= (desc_tab - 1))
		{
			putchar('\n');
			len = 0;
		}
		while (len < desc_tab)
		{
			putchar(' ');
			len++;
		}
		printf("%s\n", opt_list[i].arg.help);
	}
#endif
}
