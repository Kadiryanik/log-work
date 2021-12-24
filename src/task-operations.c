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
#define GET_TIMESTAMP() (unsigned int)time(NULL);

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
static int get_line_count(const FILE * const file)
{
	char c = 0;
	int count = 0;
	FILE *fp = (FILE *)file;

	if (fp == NULL) {
		goto out;
	}
	for (c = getc(fp); c != EOF; c = getc(fp)) {
		if (c == '\n') count++;
	}

out:
	return count;
}

/*------------------------------------------------------------------------------*/
static unsigned int get_duration(const task_t * const task)
{
	unsigned int duration = 0;

	if (task == NULL || task->end_timestamp == 0) {
		goto out;
	}
	duration = task->end_timestamp - task->start_timestamp;
out:
	return duration;
}

/*------------------------------------------------------------------------------*/
static int get_formatted_duration(unsigned int duration, char **formatted)
{
#define DURATION_MIN	60	/* 60 */
#define DURATION_HOUR	3600	/* 60 * 60    = 3600 */
#define DURATION_DAY	28800	/*  8 * 3600  = 28800  -> 8 hours 1 day */
#define DURATION_WEEK	144000	/*  5 * 28800 = 144000 -> 5 day 1 week */
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
	ret = -1;

success:
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_start(const char *file_name, const char *task_name)
{
	int ret = 0;
	char *formatted_time = NULL;
	FILE *file = NULL;
	task_t task = { 0, 0, NULL };

	util_fite(((file = fopen(file_name, "a")) == NULL),
			LOG_ERR("File open failed!\n"));

	task.start_timestamp = GET_TIMESTAMP();
	task.name = task_name ? strdup(task_name) : strdup("TEST"); /* TODO: generate */

	fprintf(file, "%s %u %u\n", task.name, task.start_timestamp, task.end_timestamp);

	util_fie(get_formatted_time(task.start_timestamp, &formatted_time));
	LOG_INFO("Task: %s " DGREEN "STARTED" NORM " successfully started at %s\n", task.name, formatted_time);

	goto success;

fail:
	ret = -1;

success:
	if (file) fclose(file);
	sfree(task.name);
	sfree(formatted_time);
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_stop(const char *file_name, const char *task_name)
{
	int ret = 0, i = 0, task_count = 0;
	char *start_formatted_time = NULL, *end_formatted_time = NULL;
	FILE *file = NULL;
	task_t *tasks = NULL;
	char buffer[64] = { 0 };

	util_fite(((file = fopen(file_name, "r+")) == NULL),
			LOG_ERR("File open failed!\n"));

	util_fit(((task_count = get_line_count(file)) == 0));
	util_fit(((tasks = calloc(task_count, sizeof(task_t))) == NULL));

	fseek(file, 0, SEEK_SET); /* Move the file pointer to the start */
	for (i = 0; i < task_count; i++) {
		if (fscanf(file, "%s %u %u\n", buffer, &tasks[i].start_timestamp, &tasks[i].end_timestamp) < 3) {
			LOG_ERR("File read failed!\n");
			goto fail;
		}
		util_fit(((tasks[i].name = strdup(buffer)) == NULL));
	}

	fseek(file, 0, SEEK_SET); /* Move the file pointer to the start */
	for (i = 0; i < task_count; i++) {
		/* if there is task_name and doesn't match with current OR
		 * current task already stoped then store old settings */
		if ((task_name && strcmp(task_name, tasks[i].name) != 0) ||
			tasks[i].end_timestamp != 0) {
			fprintf(file, "%s %u %u\n", tasks[i].name, tasks[i].start_timestamp, tasks[i].end_timestamp);
			continue;
		}
		tasks[i].end_timestamp = GET_TIMESTAMP();

		fprintf(file, "%s %u %u\n", tasks[i].name, tasks[i].start_timestamp, tasks[i].end_timestamp);
		util_fie(get_formatted_time(tasks[i].start_timestamp, &start_formatted_time));
		util_fie(get_formatted_time(tasks[i].end_timestamp, &end_formatted_time));
		LOG_INFO("Task[%d]: " DGREEN "STOPED" NORM " %s from %s to %s = %u\n",
				i, tasks[i].name, start_formatted_time, end_formatted_time,
				get_duration(&tasks[i]));
		sfree(start_formatted_time);
		sfree(end_formatted_time);
	}

	goto success;

fail:
	ret = -1;

success:
	if (file) fclose(file);
	for (i = 0; i < task_count; i++) {
		sfree(tasks[i].name);
	}
	sfree(tasks);
	sfree(start_formatted_time);
	sfree(end_formatted_time);
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_switch(const char *file_name, const char *task_name)
{
	int ret = 0;
	char *name = NULL;

	util_fie(task_stop(file_name, NULL));

	name = task_name ? strdup(task_name) : strdup("TEST"); /* TODO: generate */
	util_fie(task_start(file_name, name));

	LOG_INFO("Task: " DGREEN "SWITCHED" NORM " to %s\n", name);
	goto success;

fail:
	ret = -1;

success:
	sfree(name);
	return ret;
}

/*------------------------------------------------------------------------------*/
int task_print(const char *file_name, const char *task_name, unsigned char all)
{
	int ret = 0, i = 0;
	char *duration_str = NULL;
	unsigned int duration = 0, total_duration = 0;
	char *start_formatted_time = NULL, *end_formatted_time = NULL;
	FILE *file = NULL;
	task_t task = { 0, 0, NULL };
	char buffer[64] = { 0 };

	util_fite(((file = fopen(file_name, "r")) == NULL),
			LOG_ERR("File open failed!\n"));

	while (fscanf(file, "%s %u %u\n", buffer, &task.start_timestamp, &task.end_timestamp) == 3) {
		if (task_name && strcmp(task_name, buffer) != 0) {
			continue;
		}
		util_fie(get_formatted_time(task.start_timestamp, &start_formatted_time));
		if (task.end_timestamp == 0) {
			LOG_INFO("Task[%d]: " LRED "NOT-DONE" NORM " %s since %u %s\n",
					i, buffer, task.start_timestamp, start_formatted_time);
		} else if (all || task_name == NULL) {
			duration = get_duration(&task);
			util_fie(get_formatted_duration(duration, &duration_str));
			util_fie(get_formatted_time(task.end_timestamp, &end_formatted_time));

			LOG_INFO("Task[%d]: " DGREEN "DONE" NORM " %s from %s to %s = [%s]\n",
					i, buffer, start_formatted_time, end_formatted_time, duration_str);
			sfree(duration_str);
			sfree(end_formatted_time);
		} else {
			duration = get_duration(&task);
		}
		total_duration += duration;
		sfree(start_formatted_time);
		i++;
	}
	if (task_name) {
		util_fie(get_formatted_duration(total_duration, &duration_str));
		LOG_INFO(DGREEN "TotalDuration" NORM " for %s = [%s]\n", task_name, duration_str);
	}

	goto success;

fail:
	sfree(start_formatted_time);
	sfree(end_formatted_time);
	ret = -1;

success:
	if (file) fclose(file);
	sfree(duration_str);
	return ret;
}
