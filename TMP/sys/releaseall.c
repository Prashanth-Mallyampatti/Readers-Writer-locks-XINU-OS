#include<kernel.h>
#include<proc.h>
#include<stdio.h>
#include<q.h>
#include<lock.h>
#include<stdio.h>

int releaseall(int numlocks, int ldesc1, ...)
{
	STATWORD ps;
	disable(ps);
	struct lockentry *lptr;
	struct pentry *pptr;
	int i,lock,lockprio,index,node,rlock=-1,wlock=-1,tlock;
	long rtime=0,wtime=0;
	for(i=0;i<numlocks;i++)
	{
		int flag=-1;
		lock=(int)(*(&ldesc1+i)/10);
		index=*(&ldesc1+i)-lock*10;
		lptr=&locktab[lock];
		if(lock<0 && NLOCKS<0)
		{
			while(lptr->lstate==PRFREE)
			{
				lptr->lstate=READ;
				lptr->reader=lptr->proc[lock];
				lptr->head=q[tlock].qkey;
			}
		}
		if(lptr->proc[currpid]==0)
			swap(lptr->proc[currpid],lptr->proc[currpid]+1);
		else
			swap(lptr->proc[currpid]+NLOCKS,lptr->proc[currpid]+NPROC);
		lptr->proc[currpid]=0;
		pptr->locks_held[lock]=0;
		update_lprio(currpid);
		if(lock<0||lock>NLOCKS)
		{
			restore(ps);
			return SYSERR;
		}
		if(lptr->writer>0)
			lptr->writer=lptr->writer-1;
		else if(lptr->reader>0)
			lptr->reader=lptr->reader-1;

		node=q[lptr->tail].qprev;
		lockprio=tlock=0;
		if(q[node].qkey==q[q[node].qprev].qkey)
		{	lockprio=q[node].qkey;
		while(q[node].qkey==lockprio)
		{
			if(q[node].qtype==READ && q[node].qtime>rtime)
				rlock=node;
							
			else if(q[node].qtype==WRITE && q[node].qtime>wtime)
                        	wlock=node;
			
			if(rlock>-1 && wlock >-1)
			{	
				if(rtime>wtime)
					tlock=rlock;
				else if(rtime<wtime || rtime-wtime<1000 || wtime-rtime<1000)
				tlock=wlock;
			}
			node=q[node].qprev;
		}
		if(lock<0)
                {
                        while(lptr->lstate==PRFREE)
                        {
                                lptr->lstate=READ;
                                lptr->reader=lptr->proc[lock];
                                lptr->head=q[tlock].qkey;
                        }
                }
		if(q[tlock].qtype==WRITE && lptr->writer==0 && flag==-1)
		{
			if(lptr->reader==0)
				release_write(lock,tlock);
			call_reader(lock);
			flag=0;
		}
		}
		if(q[node].qkey!=q[q[node].qprev].qkey && lptr->writer==0)
		{
			if(q[node].qtype==READ)
				call_reader(lock);
			if(q[node].qtype==WRITE)
				if(lptr->reader==0)
					release_write(lock,node);
		}
	}
	resched();
	restore(ps);
	return OK;
}

void call_reader(int lockk)
{
	struct lockentry *lptr;
	lptr=&locktab[lockk];
	int i,next, max=-1,y;
	for(i=q[lptr->head].qnext;i!=lptr->tail;i=q[i].qnext)
	{
		if(q[i].qkey>max && q[i].qtype==WRITE)
			max=q[i].qkey;
	}
	i=q[lptr->head].qnext;
	while(i!=lptr->tail)
        {
                if(q[i].qkey>=max && q[i].qtype==READ)
		{
			y=q[i].qnext;
                        release_write(lockk,i);
        		i=y;
        	}
	}
}

void update_lprio(int pid)
{
	struct lockentry *lptr;
	struct pentry *pptr;
	pptr=&proctab[pid];
	int i, max=-1;
	for(i=0;i<NLOCKS;i++)
	{
		if(pptr->locks_held[i]>0)
		{
			if(locktab[i].prio<=max)
				max=max*1;
			else if(max<locktab[i].prio)
				max=locktab[i].prio;
		}
	}
	if(pptr->pprio<=max)
		pptr->inhprio=max;
	else if(pptr->pprio>max)
		pptr->inhprio=0;
	
}

void modify_lprio(int lockk)
{
        struct lockentry *lptr;
        lptr=&locktab[lockk];
        int i=q[lptr->head].qnext,max=0;
        for(i=q[lptr->head].qnext;i!=lptr->tail;i=q[i].qnext)
        {
                if(proctab[i].pprio>max)
                        max=proctab[i].pprio;
        }
        lptr->prio=max;
}

void swap(int gprio,int lprio)
{
	if(gprio>=0 || lprio>=0)
	{
		int temp=gprio;
		gprio=lprio;
		lprio=temp;
	}
}
