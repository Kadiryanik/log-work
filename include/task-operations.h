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
	unsigned int start_timestamp;
	unsigned int end_timestamp;
	char *name;
} task_t;

int task_start(const char *, const char *);
int task_stop(const char *, const char *);
int task_switch(const char *, const char *);
int task_print(const char *, const char *, unsigned char);

#endif /* TASK_OPER_H_ */
