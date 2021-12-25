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

#ifndef TASK_OPER_H_
#define TASK_OPER_H_

typedef struct task {
	unsigned int start_ts;	/* when task started as timestamp */
	unsigned int end_ts;	/* when task ended as timestamp, stores 0 if task is not done */
	char *name;		/* task name */
} task_t;

int task_start(const char *, const char *);
int task_stop(const char *, const char *);
int task_switch(const char *, const char *);
int task_change(const char *, const char *, const char *);
int task_remove(const char *, const char *, int);
int task_print(const char *, const char *, unsigned char);

#endif /* TASK_OPER_H_ */
