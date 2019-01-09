/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include<lock.h>
/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	//disable(ps);
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev,i,temp;
	struct lockentry *lptr;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
							/* fall through	*/
	case PRLOCK: lptr=&locktab[pptr->lproc];
			if(lptr->proc[currpid]==0)
				swap(lptr->proc[currpid]+NLOCKS,lptr->proc[currpid]);
			else
				swap(lptr->proc[currpid]+NLOCKS,lptr->proc[currpid]+NPROC);
			
			dequeue(pid);
			locktab[pptr->lproc].proc[pid]=0;
			modify_lprio(pptr->lproc);	
			int i;
			for(i=0;i<NPROC;i++)
			{
				if(lptr->proc[i]>0)
					update_lprio(i);
			}		
			pptr->pstate=PRFREE;
			break;
			
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
