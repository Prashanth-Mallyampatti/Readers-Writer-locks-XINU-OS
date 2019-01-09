#include<conf.h>
#include<kernel.h>
#include<stdio.h>
#include<proc.h>
#include<q.h>
#include<lock.h>

struct lockentry locktab[NLOCKS];
int next_lock;
int num_lock;
void linit()
{
	struct lockentry *lptr;
	next_lock=NLOCKS-1;
	num_lock=0;
	int i,j;
	for(i=0;i<NLOCKS;i++)
	{
		lptr=&locktab[i];
		lptr->lstate=LFREE;
		lptr->head=newqueue();
		lptr->prio=-1;
		lptr->tail=lptr->head+1;
		for(j=0;j<NPROC;j++)
			lptr->proc[j]=0;
	}
}
