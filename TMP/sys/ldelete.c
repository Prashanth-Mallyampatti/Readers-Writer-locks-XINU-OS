#include<conf.h>
#include<kernel.h>
#include<stdio.h>
#include<q.h>
#include<lock.h>
#include<proc.h>
int ldelete(int lockdescriptor)
{
	STATWORD ps;
	disable(ps);	
	int index,temp,pid,lock;
	temp=lockdescriptor/10;
	lock=temp;
	index=(temp-lock)*10;
	struct lockentry *ltab;
	ltab=&locktab[lock];
	if(lock<0||ltab->lstate==LFREE||lock>NLOCKS)
	{
		restore(ps);
		return SYSERR;
	}
	ltab->lstate=LFREE;
	ltab->prio=-1;
	ltab->proc[currpid]=-1;

	if(nonempty(ltab->head))
	{
		pid=getfirst(ltab->head);
		while(pid!=EMPTY)
		{
			proctab[pid].plock=DELETED;
			ready(pid,RESCHNO);
		}
		resched();
	}
	restore(ps);
	return OK;
}
