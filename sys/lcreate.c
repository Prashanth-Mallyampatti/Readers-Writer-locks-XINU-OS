#include<conf.h>
#include<kernel.h>
#include<q.h>
#include<proc.h>
#include<stdio.h>
#include<lock.h>
int lcreate()
{
	STATWORD ps;
	disable(ps);
	int lock,i;
	for(i=0;i<NLOCKS;i++)
	{
		lock=next_lock--;
		if(next_lock<0)
		{
			next_lock=NLOCKS-1;
			num_lock=num_lock+1;
		}	
		if(locktab[lock].lstate==LFREE)
		{
			locktab[lock].lstate=LUSED;
			locktab[lock].reader=0;
			locktab[lock].writer=0;
			restore(ps);
			return ((lock*10)+num_lock);
		}
	}
	restore(ps);
	return SYSERR;
}
