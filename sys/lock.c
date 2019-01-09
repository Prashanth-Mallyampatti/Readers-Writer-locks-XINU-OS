#include<conf.h>
#include<kernel.h>
#include<stdio.h>
#include<q.h>
#include<proc.h>
#include<lock.h>

int lock(int ldes1, int type, int priority)
{
	STATWORD ps;
	disable(ps);
	if(ldes1/10<0 || NLOCKS<ldes1/10)
	{
		restore(ps);
		return SYSERR;
	}
	struct lockentry *lptr,*lptr1;
        struct pentry *pptr;
	int lock=ldes1/10;
	lptr=lptr1=&locktab[lock];
	pptr=&proctab[currpid];
	if(lptr->lstate==LFREE)
	{
		restore(ps);
		return SYSERR;
	}
	int index=ldes1,i,flag=0,rc,wc;
	for(i=0;i<10;i++)
		index=index-lock;
	
if(i==10)		//edit this
                {
                        if(num_lock!=index)
                        {
                                restore(ps);
                                return SYSERR;
                        }
                }
	if(lock<0 && lptr->lstate==PRFREE)
	{
		lptr->lstate=READ;	
		lptr->reader=lptr->proc[lock];
	}
	rc=lptr->reader;
	wc=lptr->writer;
	if(rc<0 || wc<0)
	{
		restore(ps);
		return SYSERR;
	}

	if(wc==0 && rc!=0 && type==WRITE)
		flag=1;
	if(wc==0 && rc!=0 && type==READ)
	{
		int nlock=q[lptr->head].qnext;
		while(priority<q[nlock].qkey)
		{
			if(q[nlock].qtype==WRITE)
				flag=1;
			nlock=q[nlock].qnext;
		}
	}
	if(wc!=0 && rc==0)
		flag=1;
	if(wc==0 && rc==0)
		flag=0;

	if(flag==0)
	{
		lptr->proc[currpid]=1;
		proctab[currpid].locks_held[lock]=1;
		update_lprio(currpid);
		if(type==READ)
			lptr->reader++;
		if(type==WRITE)
			lptr->writer++;
		restore(ps);
		return OK;
	}
	else if(flag==1)
	{
		pptr->pstate=PRLOCK;
		pptr->lproc=lock;
		pptr->plock=OK;
		insert(currpid,lptr->head,priority);
		q[currpid].qtype=type;
		q[currpid].qtime=ctr1000;
		modify_lprio(lock);
		int i;
		for(i=0;i<NPROC;i++)
		{	
			if(lptr1->proc[i]>0)
				update_lprio(i);
		}
		resched();
		restore(ps);
		return pptr->plock;
	}
	if(lptr->proc[currpid]==0)
		swap(lptr->proc[currpid],lptr->proc[currpid]+1);
	else
		swap(lptr->proc[currpid]+NLOCKS,lptr->proc[currpid]+NPROC);
	restore(ps);
	return OK;

}
void release_write(int lock,int node)
{
	struct lockentry *lptr;
	struct pentry *pptr;
	lptr=&locktab[lock];
	pptr=&proctab[currpid];
	lptr->proc[node]=1;
	pptr->locks_held[lock]=1;
	if(q[node].qtype==READ)
		lptr->reader=lptr->reader+1;
	if(q[node].qtype==WRITE)
		lptr->writer=lptr->writer+1;
	modify_lprio(lock);
	int i;
	for(i=0;i<NPROC;i++)
		if(lptr->proc[i]>0)
			update_lprio(i);		
	
	dequeue(node);
	ready(node,RESCHNO);
}
