/**
 * \file
 *	Task Operations
 *
 * \author
 *	Kadir Yanık <kdrynkk@gmail.com>
 *
 * \date
 *	12.2021
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "util.h"
#include "task-operations.h"

/*------------------------------------------------------------------------------*/
#define MAX_TASK_NAME	32
#define GET_TIMESTAMP()	(unsigned int)time(NULL)

#define _PUT_INDENT(name, padding) name, (indent = (int)(padding - strlen(name))) > 0 ? indent : 0, ""

#define TASK_PRINT_FORMAT "[" LBLUE "%s" NORM "]%*s: "
#define PUT_INDENT(name) _PUT_INDENT(name, 20)

#define TASK_PRINT_IDX_FORMAT "[%05d]" TASK_PRINT_FORMAT
#define PUT_IDX_INDENT(name) _PUT_INDENT(name, 13)

/*------------------------------------------------------------------------------*/
static int get_formatted_time(unsigned int timestamp, char **formatted)
{
	int ret = 0;
	time_t timer;
	char buffer[26];
	struct tm* tm_info;

	util_fit((formatted == NULL));
	*formatted = NULL;

	timer = (time_t)timestamp;
	tm_info = localtime(&timer);

	strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

	util_fit(((*formatted = strdup(buffer)) == NULL));

	goto success;

fail:
	ret = -1;

success:
	return ret;
}

/*------------------------------------------------------------------------------*/
static int get_line_count(const char *file_name)
{
	char c = 0;
	int count = 0;
	FILE *file = NULL;

	util_fite(((file = fopen(file_name, "r")) == NULL),
			LOG_ERR("File open failed!\n"));

	if (file == NULL) {
		goto fail;
	}

	for (c = getc(file); c != EOF; c = getc(file)) {
		if (c == '\n') count++;
	}

	goto success;

fail:
	count = -1;

success:
	if (file) fclose(file);
	return count;
}

/*------------------------------------------------------------------------------*/
static unsigned int get_duration(const task_t * const task)
{
	unsigned int duration = 0;

	if (task == NULL || task->end_ts == 0) {
		goto out;
	}
	duration = task->end_ts - task->start_ts;
out:
	return duration;
}

/*------------------------------------------------------------------------------*/
static int get_formatted_duration(unsigned int duration, char **formatted)
{
#define DURATION_MIN	60	/* 60 */
#define DURATION_HOUR	3600	/* 60 * 60    = 3600 */
#define DURATION_DAY	28800	/*  8 * 3600  = 28800  -> 8 hours = 1 day */
#define DURATION_WEEK	144000	/*  5 * 28800 = 144000 -> 5 day   = 1 week */
	unsigned int w, d, h, m;
	int ret = 0;

	if (formatted == NULL) {
		goto fail;
	}

	*formatted = NULL;

	w = duration / (DURATION_WEEK);
	duration -= w * DURATION_WEEK;

	d = duration / (DURATION_DAY);
	duration -= d * DURATION_DAY;

	h = duration / DURATION_HOUR;
	duration -= h * DURATION_HOUR;

	m = duration / DURATION_MIN;
	duration -= m * DURATION_MIN;

	if (asprintf(formatted, "%uw %ud %uh %um %us", w, d, h, m, duration) < 0) {
		*formatted = NULL;
		goto fail;
	}

	goto success;

fail:
	LOG_ERR("failed!\n");
	ret = -1;

success:
	return ret;
}

/*------------------------------------------------------------------------------*/
static int generate_task_name(unsigned int max, char **name)
{
	int ret = 0;

	util_fit((name == NULL));

	util_fite((asprintf(name, "Task-%u", max) < 0), { *name = NULL; });

	goto success;

fail:
	LOG_ERR("failed!\n");
	ret = -1;

success:
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_start(const char *file_name, const char *task_name)
{
	int ret = 0, indent = 0, task_count = 0;
	char *time_str = NULL;
	FILE *file = NULL;
	task_t task = { 0, 0, NULL };

	util_fit((!file_name));

	util_fite(((file = fopen(file_name, "a")) == NULL),
			LOG_ERR("File open failed!\n"));

	task.start_ts = GET_TIMESTAMP();
	util_fit(((task_count = get_line_count(file_name)) < 0));
	if (task_name) {
		task.name = strdup(task_name);
	} else {
		util_fie(generate_task_name(task_count, &task.name));
	}

	fprintf(file, "%s %u %u\n", task.name, task.start_ts, task.end_ts);

	util_fie(get_formatted_time(task.start_ts, &time_str));
	LOG_INFO(TASK_PRINT_IDX_FORMAT DGREEN "STARTED" NORM " [%s]\n", task_count, PUT_IDX_INDENT(task.name), time_str);

	goto success;

fail:
	LOG_ERR("failed!\n");
	ret = -1;

success:
	if (file) fclose(file);
	sfree(task.name);
	sfree(time_str);
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_stop(const char *file_name, const char *task_name)
{
	int ret = 0, i = 0, task_count = 0, indent = 0;
	char buffer[MAX_TASK_NAME] = { 0 }, *start_time_str = NULL,
	     *end_time_str = NULL, *duration_str = NULL;
	FILE *file = NULL;
	task_t *tasks = NULL;

	util_fit((!file_name));

	util_fite(((file = fopen(file_name, "r+")) == NULL), LOG_ERR("File open failed!\n"));

	util_fit(((task_count = get_line_count(file_name)) <= 0));
	util_fit(((tasks = calloc(task_count, sizeof(task_t))) == NULL));

	for (i = 0; i < task_count; i++) {
		util_fite((fscanf(file, "%s %u %u\n", buffer, &tasks[i].start_ts, &tasks[i].end_ts) < 3),
				LOG_ERR("File read failed!\n"));
		util_fit(((tasks[i].name = strdup(buffer)) == NULL));
	}

	fseek(file, 0, SEEK_SET); /* Move the file pointer to the start */
	for (i = 0; i < task_count; i++) {
		/* if there is task_name and doesn't match with current OR
		 * current task already stoped then store old settings */
		if ((task_name && strcmp(task_name, tasks[i].name) != 0) ||
			tasks[i].end_ts != 0) {
			fprintf(file, "%s %u %u\n", tasks[i].name, tasks[i].start_ts, tasks[i].end_ts);
			continue;
		}
		tasks[i].end_ts = GET_TIMESTAMP();

		/* write to file */
		fprintf(file, "%s %u %u\n", tasks[i].name, tasks[i].start_ts, tasks[i].end_ts);

		/* Get and log formatted datas */
		util_fie(get_formatted_time(tasks[i].start_ts, &start_time_str));
		util_fie(get_formatted_time(tasks[i].end_ts, &end_time_str));
		util_fie(get_formatted_duration(get_duration(&tasks[i]), &duration_str));

		LOG_INFO(TASK_PRINT_IDX_FORMAT DGREEN "STOPED" NORM " [%s] -> [%s] = [%s]\n", i,
				PUT_IDX_INDENT(tasks[i].name), start_time_str, end_time_str, duration_str);

		sfree(start_time_str);
		sfree(end_time_str);
		sfree(duration_str);
	}

	goto success;

fail:
	LOG_ERR("failed!\n");
	sfree(start_time_str);
	sfree(end_time_str);
	sfree(duration_str);
	ret = -1;

success:
	if (file) fclose(file);
	if (tasks) {
		for (i = 0; i < task_count; i++) {
			sfree(tasks[i].name);
		}
		sfree(tasks);
	}
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_switch(const char *file_name, const char *task_name)
{
	int ret = 0, indent = 0, task_count = 0;
	char *name = NULL;

	util_fit((!file_name));

	util_fie(task_stop(file_name, NULL));

	util_fit(((task_count = get_line_count(file_name)) < 0));
	if (task_name) {
		name = strdup(task_name);
	} else {
		util_fie(generate_task_name(task_count, &name));
	}
	util_fie(task_start(file_name, name));

	LOG_INFO(TASK_PRINT_IDX_FORMAT "successfully " DGREEN "SWITCHED" NORM "\n", task_count, PUT_IDX_INDENT(name));
	goto success;

fail:
	LOG_ERR("failed!\n");
	ret = -1;

success:
	sfree(name);
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_change(const char *file_name, const char *task_name, const char *new_task_name)
{
	int ret = 0, i = 0, task_count = 0, indent = 0, exist = 0;
	char buffer[MAX_TASK_NAME] = { 0 };
	FILE *file = NULL;
	task_t *tasks = NULL;

	util_fit((!file_name || !task_name || !new_task_name));

	util_fite(((file = fopen(file_name, "r+")) == NULL), LOG_ERR("File open failed!\n"));

	util_fit(((task_count = get_line_count(file_name)) <= 0));
	util_fit(((tasks = calloc(task_count, sizeof(task_t))) == NULL));

	for (i = 0; i < task_count; i++) {
		util_fite((fscanf(file, "%s %u %u\n", buffer, &tasks[i].start_ts, &tasks[i].end_ts) < 3),
				LOG_ERR("File read failed!\n"));
		util_fit(((tasks[i].name = strdup(buffer)) == NULL));
		if (exist == 0 && strcmp(buffer, task_name) == 0) {
			exist = 1; /* at least one of them will change */
		}
	}

	util_site((exist == 0), LOG_INFO(TASK_PRINT_FORMAT "Doesnt exist!\n", PUT_INDENT(task_name)));

	fseek(file, 0, SEEK_SET); /* Move the file pointer to the start */
	for (i = 0; i < task_count; i++) {
		if (strcmp(task_name, tasks[i].name) != 0) {
			fprintf(file, "%s %u %u\n", tasks[i].name, tasks[i].start_ts, tasks[i].end_ts);
			continue;
		}

		/* write with new task name to file */
		fprintf(file, "%s %u %u\n", new_task_name, tasks[i].start_ts, tasks[i].end_ts);
	}
	LOG_INFO(TASK_PRINT_FORMAT DGREEN "CHANGED" NORM " to [%s]\n", PUT_INDENT(task_name), new_task_name);

	goto success;

fail:
	LOG_ERR("failed!\n");
	ret = -1;

success:
	if (file) fclose(file);
	if (tasks) {
		for (i = 0; i < task_count; i++) {
			sfree(tasks[i].name);
		}
		sfree(tasks);
	}
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_remove(const char *file_name, const char *task_name, int index)
{
	int ret = 0, i = 0, task_count = 0, indent = 0, exist = 0;
	char buffer[MAX_TASK_NAME] = { 0 };
	FILE *file = NULL, *temp_file = NULL;
	task_t *tasks = NULL;

	util_fit((!file_name));

	util_fite(((file = fopen(file_name, "r")) == NULL), LOG_ERR("File open failed!\n"));

	util_fit(((task_count = get_line_count(file_name)) <= 0));

	if (index >= 0) {
		if (index >= task_count) {
			LOG_ERR("Invalid index!\n");
			goto fail;
		}
		exist = 1; /* removing with index so we can set this flag */
	} else if (!task_name) {
		LOG_ERR("There isnt index or task name!\n");
		goto fail;
	}

	util_fit(((tasks = calloc(task_count, sizeof(task_t))) == NULL));

	for (i = 0; i < task_count; i++) {
		util_fite((fscanf(file, "%s %u %u\n", buffer, &tasks[i].start_ts, &tasks[i].end_ts) < 3),
				LOG_ERR("File read failed!\n"));
		util_fit(((tasks[i].name = strdup(buffer)) == NULL));
		if (exist == 0 && task_name && strcmp(buffer, task_name) == 0) {
			exist = 1; /* at least one of them will change */
		}
	}

	util_site((exist == 0), LOG_INFO(TASK_PRINT_FORMAT "Doesnt exist!\n", PUT_INDENT(task_name)));

	util_fite(((temp_file = fopen("__tempfile__", "w")) == NULL), LOG_ERR("File open failed!\n"));
	for (i = 0; i < task_count; i++) {
		/* If this will not remove then write to file */
		if ((index >= 0) ? (index != i) : (strcmp(task_name, tasks[i].name) != 0)) {
			fprintf(temp_file, "%s %u %u\n", tasks[i].name, tasks[i].start_ts, tasks[i].end_ts);
			continue;
		}

		LOG_INFO(TASK_PRINT_IDX_FORMAT DGREEN "REMOVED" NORM "\n", i, PUT_INDENT(tasks[i].name));
	}

	rename("__tempfile__", file_name);
	goto success;

fail:
	ret = -1;

success:
	if (file) fclose(file);
	if (temp_file) fclose(temp_file);
	if (tasks) {
		for (i = 0; i < task_count; i++) {
			sfree(tasks[i].name);
		}
		sfree(tasks);
	}
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_print(const char *file_name, const char *task_name, unsigned char total_only)
{
	int ret = 0, i = 0, indent = 0;
	char buffer[MAX_TASK_NAME] = { 0 }, *duration_str = NULL,
	     *start_time_str = NULL, *end_time_str = NULL;
	unsigned int duration = 0, total_duration = 0;
	FILE *file = NULL;
	task_t task = { 0, 0, NULL };

	util_fit((!file_name));

	util_fite(((file = fopen(file_name, "r")) == NULL),
			LOG_ERR("File open failed!\n"));

	while (fscanf(file, "%s %u %u\n", buffer, &task.start_ts, &task.end_ts) == 3) {
		if (task_name && strcmp(task_name, buffer) != 0) {
			continue;
		}
		util_fie(get_formatted_time(task.start_ts, &start_time_str));
		if (task.end_ts == 0) {
			util_fie(get_formatted_duration(GET_TIMESTAMP() - task.start_ts, &duration_str));
			LOG_INFO(TASK_PRINT_IDX_FORMAT LRED "NOT-DONE" NORM " [%s] -> [NOW] = [%s]\n",
					i, PUT_IDX_INDENT(buffer), start_time_str, duration_str);
		} else {
			duration = get_duration(&task);
			if (!total_only) {
				util_fie(get_formatted_duration(duration, &duration_str));
				util_fie(get_formatted_time(task.end_ts, &end_time_str));

				LOG_INFO(TASK_PRINT_IDX_FORMAT DGREEN "DONE" NORM " [%s] -> [%s] = [%s]\n",
						i, PUT_IDX_INDENT(buffer), start_time_str, end_time_str, duration_str);
				sfree(end_time_str);
			}
		}
		total_duration += duration;
		sfree(start_time_str);
		sfree(duration_str);
		i++;
	}
	if (task_name) {
		util_fie(get_formatted_duration(total_duration, &duration_str));
		LOG_INFO(TASK_PRINT_FORMAT DGREEN "TOTAL" NORM " = [%s]\n", PUT_INDENT(task_name), duration_str);
	}

	goto success;

fail:
	LOG_ERR("failed!\n");
	sfree(start_time_str);
	sfree(end_time_str);
	ret = -1;

success:
	if (file) fclose(file);
	sfree(duration_str);
	return ret;
}
