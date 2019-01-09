/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include<lock.h>
unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	/* no switch needed if current process priority higher than next*/

	if((optr=&proctab[currpid])->pstate==PRCURR)
	{
		if(isempty(rdyhead))
			return OK;
	}
	int pid=q[rdyhead].qnext;
	int max_prio=-1,max_pid=-1,temp=0;
	while(pid!=rdytail)
	{
		max_prio=temp;
		max_pid=pid;
		pid=q[pid].qnext;
	}
	pid=q[rdyhead].qnext;
	while(pid!=rdytail)
	{
		if(proctab[pid].inhprio==0 && proctab[pid].pprio>max_prio)
		{
			max_prio=proctab[pid].pprio;
			max_pid=pid;
		}
		if(proctab[pid].inhprio>max_prio)
		{
			max_prio=proctab[pid].inhprio;
                        max_pid=pid;
		}
		pid=q[pid].qnext;

	}
	if(( (optr->inhprio==0 && max_pid<optr->pprio) || (optr->inhprio!=0 && max_pid<optr->inhprio) ) && optr->pstate==PRCURR)
		return OK;

	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}

	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */
	currpid=max_pid;
	dequeue(max_pid);
	nptr=&proctab[currpid];
	
//	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
