/**
 * \file
 *	Log Work
 *
 * \author
 *	Kadir Yanık <kdrynkk@gmail.com>
 *
 * \date
 *	12.2021
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "util.h"
#include "task-operations.h"

/*------------------------------------------------------------------------------*/
#define DEFAULT_DB_FILE "db.txt"
#define OPTIONAL_ARG ((optarg == NULL && optind < argc && argv[optind][0] != '-') ? argv[optind++] : optarg)

/*------------------------------------------------------------------------------*/
enum log_work_options {
	OPT_DB = 0,
	OPT_START,
	OPT_STOP,
	OPT_SWITCH,
	OPT_CHANGE,
	OPT_PRINT,
	OPT_PRINT_ALL,
	OPT_MAX,
};

/*------------------------------------------------------------------------------*/
static void usage(const char *);

/*------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	uint8_t option = 0;
	int i, c, ret = 0;
	char *db_file_name = NULL, *old_name = NULL;
	char *opt_arg[OPT_MAX];

	struct option options[] = {
		{ "database",	required_argument,	NULL,	'd' },
		{ "change",	required_argument,	NULL,	'c' },
		{ "start",	optional_argument,	NULL,	'j' },
		{ "stop",	optional_argument,	NULL,	'q' },
		{ "switch",	optional_argument,	NULL,	's' },
		{ "print",	optional_argument,	NULL,	'p' },
		{ "print-all",	optional_argument,	NULL,	'P' },
		{ NULL,		0,			0,	0 }
	};

	for (i = 0; i < OPT_MAX; i++) {
		opt_arg[i] = NULL;
	}

	while ((c = getopt_long(argc, argv, "d:c:j::q::s::p::P::h", options, NULL)) > 0) {
		switch (c) {
		case 'd':
			printf("database file %s\n", optarg);
			break;
		case 'c':
			option = OPT_CHANGE;
			if (optind + 1 > argc) {
				LOG_ERR("Missing arguments for for option '-c, --change'\n");
				goto usage;
			}
			old_name = optarg;
			opt_arg[option] = argv[optind++];
			if (opt_arg[option][0] == '-') {
				LOG_ERR("Missing arguments for for option '-c, --change'\n");
				goto usage;
			}
			printf("optarg for %c = %s, %s\n", c, old_name, opt_arg[option]);
			break;
		case 'j':
			option = OPT_START;
			opt_arg[option] = OPTIONAL_ARG;
			break;
		case 'q':
			option = OPT_STOP;
			opt_arg[option] = OPTIONAL_ARG;
			break;
		case 's':
			option = OPT_SWITCH;
			opt_arg[option] = OPTIONAL_ARG;
			break;
		case 'p':
			option = OPT_PRINT;
			opt_arg[option] = OPTIONAL_ARG;
			break;
		case 'P':
			option = OPT_PRINT_ALL;
			opt_arg[option] = OPTIONAL_ARG;
			break;
		case 'h':
			goto usage;
		default:
			LOG_ERR("Unsupported format '%c'\n", c);
			goto usage;
		}
	}

	if (db_file_name == NULL) {
		db_file_name = DEFAULT_DB_FILE;
	}
	if (option == OPT_START) {
		util_fie(task_start(db_file_name, opt_arg[option]));
	} else if (option == OPT_STOP) {
		util_fie(task_stop(db_file_name, opt_arg[option]));
	} else if (option == OPT_SWITCH) {
		util_fie(task_switch(db_file_name, opt_arg[option]));
	} else if (option == OPT_CHANGE) {
		/* TODO: Add */
	} else if (option == OPT_PRINT) {
		util_fie(task_print(db_file_name, opt_arg[option], 0));
	} else if (option == OPT_PRINT_ALL) {
		util_fie(task_print(db_file_name, opt_arg[option], 1));
	} else {
		LOG_ERR("There isnt any task operation!\n");
		goto fail;
	}

	goto success;

usage:
	usage(argv[0]);

fail:
	ret = -1;

success:
	return ret;
}

/*------------------------------------------------------------------------------*/
void usage(const char *name) {
	fprintf(stderr, "\nUsage: %s [...]\n"
			"\tRequire-Arguments:\n"
			"\t\t-d, --database   <database-file>  database file\n"
			"\t\t-c, --change     <old-task-name> <new-task-name>\n"
			"\tOptional-Arguments:\n"
			"\t\t-j, --start      [task-name]      exp.\n"
			"\t\t-q, --stop       [task-name]      exp.\n"
			"\t\t-s, --switch     [task-name]      exp.\n"
			"\t\t-p, --print      [task-name]      prints total of related log\n"
			"\t\t-P, --print-all  [task-name]      prints all related log\n", name);
}
