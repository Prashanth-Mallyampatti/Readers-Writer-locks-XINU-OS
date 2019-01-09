#ifndef _LOCK_H_
#define _LOCK_H_

#define NLOCKS 50
#define READ 0
#define WRITE 1
#define LFREE 2
#define LUSED 3

struct lockentry
{
	int lstate;
	int ltype;
	int reader;
	int writer;
	int head;
	int tail;
	int prio;
	int proc[NPROC];
};
extern struct lockentry locktab[];
extern int next_lock;
extern int num_lock;
extern long ctr1000;

extern void update_lprio(int);
extern void modify_lprio(int);
#endif
