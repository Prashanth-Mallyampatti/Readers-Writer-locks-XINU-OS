#include<conf.h>
#include<kernel.h>
#include<stdio.h>
#include<proc.h>
#include<q.h>
#include<proc.h>
#include<lock.h>
#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }

void semtest();
void locktest();
void sem_reader(int);
void sem_writer(int);
void lock_writer(int);
void lock_reader(int);
void task1()
{
	semtest();
	kprintf("\n\nSemaphore Test OK\n");
	locktest();	
	kprintf("\n\nLock Test OK\n");
}
void semtest()
{
	kprintf("\n************* Semaphore Testing **************\n");
	int semo,p1,p2,p3;
	semo=screate(1);
	p1=create(sem_reader,2000,10,"A",1,semo);
	p2=create(sem_reader,2000,20,"B",1,semo);
	p3=create(sem_writer,2000,30,"C",1,semo);
	
	kprintf("\nStarting A(reader process):\n");
	resume(p1);
	sleep(1);
	kprintf("\n\nStarting B(reader process):\n");
        resume(p2);
        sleep(1);
	kprintf("\n\nStarting C(writer process):\n");
        resume(p3);
        sleep(5);
}

void sem_reader(int sem)
{
	kprintf("Process %s priority: %d",proctab[currpid].pname,getprio(currpid));
	wait(sem);
	kprintf("\nProcess %s acquired semaphore, priority: %d",proctab[currpid].pname,getprio(currpid));
	sleep(1);
        kprintf("\nPriority of %s: %d(no ramp up)",proctab[currpid].pname,getprio(currpid));
        kprintf("\nProcess %s releasing lock",proctab[currpid].pname);
	signal(sem);
}
void sem_writer(int sem)
{
	kprintf("Process %s priority: %d",proctab[currpid].pname,getprio(currpid));
	wait(sem);
	kprintf("\nProcess %s acquired semaphore, priority: %d",proctab[currpid].pname,getprio(currpid));
	sleep(1);
	kprintf("\nProcess %s releasing semaphore");
	signal(sem);
}

void locktest()
{
	kprintf("\n\n******* Locks with Priority Inversion ********\n");
	int lock,p1,p2,p3;
	lock=lcreate();
	assert(lock!=SYSERR, "Test Failed");
	
	p1=create(lock_reader,2000,10,"A",1,lock);
	p2=create(lock_reader,2000,20,"B",1,lock);
	p3=create(lock_writer,2000,30,"C",1,lock);	
	
	kprintf("\nStarting A(reader process):\n");
	resume(p1);
	sleep(1);
	kprintf("\n\nStarting B(reader process):\n");
	resume(p2);
	sleep(1);
	kprintf("\n\nStarting C(writer process):\n");
	resume(p3);
	sleep(5);
}
void lock_reader(int lockk)
{
	kprintf("Process %s priority: %d",proctab[currpid].pname,getprio(currpid));
        lock(lockk,READ, DEFAULT_LOCK_PRIO);
        kprintf("\nProcess %s acquired lock, priority: %d",proctab[currpid].pname,getprio(currpid));
        sleep(5);
	kprintf("\nPriority od %s: %d(ramped up)",proctab[currpid].pname,getprio(currpid));
	kprintf("\nProcess %s releasing lock",proctab[currpid].pname);
        releaseall(1,lockk);
}
void lock_writer(int lockk)
{
	kprintf("Process %s priority: %d",proctab[currpid].pname,getprio(currpid));
        lock(lockk,WRITE, DEFAULT_LOCK_PRIO);
	kprintf("\nProcess %s acquired lock, priority: %d",proctab[currpid].pname,getprio(currpid));
        sleep(1);
        kprintf("\nProcess %s releasing lock",proctab[currpid].pname);
        releaseall(1,lockk);
}
