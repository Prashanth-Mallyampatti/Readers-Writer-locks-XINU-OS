#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>

#include <stdio.h>

#include <locki.h>
#include <q.h>

#define DEFAULT_LOCK_PRIO 20
#define NLOCKS 50
#define assert(x,error) if(!(x)){ \
            totalfailed += 1;\
            kprintf(error);\
            return;\
            }
int mystrncmp(char* des,char* target,int n){
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;

}
/*--------------------------------Test 1--------------------------------*/
int totalfailed = 0;
int totalpassed = 0;
int testval = 1;
void reader1 (char *msg, int lck)
{
    lock (lck, READ, DEFAULT_LOCK_PRIO);
    testval = testval*10;
    //kprintf ("  %s: acquired lock, sleep 2s.", msg);
    sleep (2);
    //kprintf ("  %s: to release lock", msg);
    testval = testval - 5;
    releaseall (1, lck);
}

void test1 ()
{
    int lck;
    int pid1;
    int pid2;

    kprintf("\nTest 1: readers can share the rwlock.\n");
    testval = 1;
    lck  = lcreate ();
    assert (lck != SYSERR,"Test 1 FAILED\n");

    pid1 = create(reader1, 2000, 20, "reader a", 2, "reader a", lck);
    pid2 = create(reader1, 2000, 20, "reader b", 2, "reader b", lck);

    resume(pid1);
    sleep(1);
    resume(pid2);
    
    sleep (5);
    ldelete (lck);
    kill(pid1);
    kill(pid2);
    assert (testval == 90,"Test 1 FAILED\n");
    kprintf ("Test 1 PASSED!\n");
    totalpassed += 1;
}


/*----------------------------------Test 2---------------------------*/
void reader2 (char *msg, int lck)
{
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        testval = testval*10;
        //kprintf ("  %s: acquired lock, sleep 2s\n", msg);
        sleep (3);
        testval = testval-10;
        //kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer2 (char *msg, int lck)
{
    lock (lck, WRITE, DEFAULT_LOCK_PRIO);
    testval = testval*10;
    //kprintf ("  %s: acquired lock, sleep 3s\n", msg);
    sleep (3);
    testval = testval-5;
    //kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
}

void test2 ()
{
        int     lck;
        int     pid1;
        int     pid2;
        testval = 1;
        kprintf("\nTest 2: write lock is mutual exclusive\n");
        lck  = lcreate ();
        assert (lck != SYSERR,"Test 2 FAILED\n");

        pid1 = create(writer2, 2000, 20, "writer2", 2, "writer", lck);
        pid2 = create(reader2, 2000, 20, "reader2", 2, "reader", lck);

        //kprintf("-start writer, then sleep 1s\n");
        resume(pid1);
        sleep (2);
        //kprintf("-start reader\n");
        resume(pid2);

        sleep (8);
        ldelete (lck);
        kill(pid1);
        kill(pid2);
        assert (testval == 40,"Test 2 FAILED\n");
        kprintf ("Test 2 PASSED!\n");
        totalpassed += 1;

}

/*-----------------------------------Test 3---------------------------*/
void test3 ()
{
        int     lck[NLOCKS];
        int last_lck;
        int     index;


        kprintf("\nTest 3: return SYSERR if no lock is available\n");
    
    //kprintf("-allocate all the locks(NLOCKS)\n");
        for (index = 0; index < NLOCKS; index++) {
            lck[index] = lcreate ();
            //kprintf("Val = %d",lck[index]);
            assert (lck[index] != SYSERR,"Test 3 FAILED\n");
        }

        //kprintf("-try to allocate one more\n");
        last_lck  = lcreate ();
        //kprintf("Val = %d",last_lck);
        for (index = 0; index < NLOCKS; index++) {
                //kprintf("val = %d",lck[index]);
                ldelete (lck[index]);
        }        
        assert (last_lck == SYSERR,"Test 3 FAILED\n");

    
    


        kprintf ("Test 3 PASSED!\n");
        totalpassed += 1;
}

/*---------------------------------Test 4--------------------------*/
void test4 ()
{
        int     lck[NLOCKS];
        int     old_lck;
        int     index;
        int ret;


        kprintf("\nTest 4: return SYSERR if lock is stale\n");
    
    //kprintf("-allocate all the locks(NLOCKS)\n");
    /*In this test case, we will first allocate all the locks, 
    then delete the first one, and create another lock. The new 
    lock will share the same location as the just deleted one*/
        for (index = 0; index < NLOCKS; index++) {
                lck[index] = lcreate ();
                //kprintf("Isempty = %d",isempty(locks[lck[index] % NLOCKS].lwriteqhead));
                //kprintf("Isempty = %d",isempty(locks[lck[index] % NLOCKS].lreadqhead));
                //kprintf("Val = %d",lck[index]);
                assert (lck[index] != SYSERR,"Test 4 FAILED\n");
        }

        //kprintf("-release the first lock, then create it again\n");
        old_lck  = lck[0];
        ldelete (lck[0]);
    
        lck[0]   = lcreate ();
        assert (lck[0] != SYSERR,"Test 4 FAILED\n");
        //kprintf("val = %d %d %d %d",old_lck,old_lck%NLOCKS,lck[0],lck[0]%NLOCKS);
        
        //kprintf("-try to acquire the old lock\n");
        ret = lock (old_lck, READ, DEFAULT_LOCK_PRIO);
        
        assert (ret == SYSERR,"Test 4 FAILED\n");
    
        
        for (index = 0; index < NLOCKS; index++) {
                //kprintf ("Deleting %d!\n",lck[index]);
                ldelete (lck[index]);
        }

        kprintf ("Test 4 PASSED!\n");
        totalpassed += 1;
}


/*----------------------------------Test 5---------------------------*/
void reader5 (char *msg, int lck)
{
    int ret;

    //kprintf ("  %s: to acquire lock, will be blocked\n", msg);
    ret = lock (lck, READ, DEFAULT_LOCK_PRIO);
    assert (ret == DELETED,"Test 5 FAILED\n");
    kprintf ("Test 5 PASSED!\n");
    totalpassed += 1;
    //kprintf ("  %s: lock deleted while waiting\n", msg);
}

void writer5 (char *msg, int lck)
{
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        //kprintf ("  %s: acquired lock, sleep 5s\n", msg);
        sleep (5);
        //kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test5 ()
{
        int     lck;
        int     pid1;
        int     pid2;

        kprintf("\nTest 5: return DELETED if lock deleted while waiting\n");
        lck  = lcreate ();
        assert (lck != SYSERR,"Test 5 FAILED\n");

        pid1 = create(writer5, 2000, 20, "writer5", 2, "writer", lck);
        pid2 = create(reader5, 2000, 20, "reader5", 2, "reader", lck);

        //kprintf("-start writer, then sleep 1s\n");
        resume(pid1);
        sleep (1);

        //kprintf("-start reader, then sleep 1s\n");
        resume(pid2);
        sleep (1);

        //kprintf("-delete the lock, reader will be waken\n");  
        ldelete (lck);

        sleep (5);
        kill(pid1);
        kill(pid2);
}

/*-----------------------------------Test 6---------------------------*/
void test6 ()
{
        int     lck[5];
        int     index;
        int ret;

        kprintf("\nTest 6: release multiple locks simultaneously\n");

        //kprintf("-create and acquire 5 locks\n");
        for (index = 0; index < 5; index++) {
                lck[index] = lcreate ();
                assert (lck[index] != SYSERR,"Test 6 FAILED\n");

                ret = lock (lck[index], READ, DEFAULT_LOCK_PRIO);
                assert (ret == OK,"Test 6 FAILED\n");
        }

        //kprintf("-release them in batches of 2 and 3\n");
        ret = releaseall (2, lck[4], lck[0]);
        assert (ret == OK,"Test 6 FAILED\n");

        ret = releaseall (3, lck[1], lck[3], lck[2]);
        assert (ret == OK,"Test 6 FAILED\n");

        for (index = 0; index < NLOCKS; index++) {
                ldelete (lck[index]);
        }

        kprintf ("Test 6 PASSED!\n");
        totalpassed += 1;
}

/*----------------------------------Test 7---------------------------*/
char output7[10];
int count7;
void reader7 (char i, int lck, int lprio)
{
        int     ret;

        //kprintf ("  %c: to acquire lock\n", i);
        lock (lck, READ, lprio);
        output7[count7++]=i;
        //kprintf ("  %c: acquired lock, sleep 3s\n", i);
        sleep (3);
        //kprintf ("  %c: to release lock\n", i);
        output7[count7++]=i;
        releaseall (1, lck);
        
}

void writer7 (char i, int lck, int lprio)
{
        //kprintf ("  %c: to acquire lock\n", i);
        lock (lck, WRITE, lprio);
        output7[count7++]=i;
        //kprintf ("  %c: acquired lock, sleep 3s\n", i);
        sleep (3);
        //kprintf ("  %c: to release lock\n", i);
        output7[count7++]=i;
        releaseall (1, lck);
        
}

void test7 ()
{
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1;

        count7 = 0;
        kprintf("\nTest 7: wait on locks with priority. Expected order of "
        "lock acquisition is: reader A, reader B, reader C, writer E, reader D\n");
        lck  = lcreate ();
        assert (lck != SYSERR,"Test 7 FAILED\n");

        rd1 = create(reader7, 2000, 20, "reader7", 3, 'A', lck, 20);
        rd2 = create(reader7, 2000, 20, "reader7", 3, 'B', lck, 30);
        rd3 = create(reader7, 2000, 20, "reader7", 3, 'C', lck, 40);
        rd4 = create(reader7, 2000, 20, "reader7", 3, 'D', lck, 20);
        wr1 = create(writer7, 2000, 20, "writer7", 3, 'E', lck, 25);
    
        //kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        //kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep (1);


        //kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
        sleep10(1);
        resume (rd3);
        sleep10(1);
        resume (rd4);


        sleep (10);
        ldelete (lck);
        kill(rd1);kill(rd2);kill(rd3);kill(rd4);kill(wr1);
        kprintf("Output is %s\n",output7);
        assert(mystrncmp(output7,"ABC...EEDD",10)==0,"Test 7 FAILED\n");
        kprintf ("Test 7 PASSED\n");
        totalpassed += 1;

}
/*----------------------------------Test 8---------------------------*/
char output8[6];
int count8;
void reader8 (char i, int lck, int lprio)
{
        int     ret;

        //kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output8[count8++]=i;
        //kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (3);
        //kprintf ("  %s: to release lock\n", msg);
        output8[count8++]=i;
        releaseall (1, lck);
        
}

void writer8 (char i, int lck, int lprio)
{
        //kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output8[count8++]=i;
        //kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (4);
        //kprintf ("  %s: to release lock\n", msg);
        output8[count8++]=i;
        releaseall (1, lck);
        
}

void test8 ()
{
        int     lck;
        int     rd1;
        int     wr1,wr2;

        count8 = 0;
        kprintf("\nTest 8: wait on locks with equal priority, waiting time diff > 1s. Expected order of "
        "lock acquisition is: writer C, reader A, writer B\n");
        lck  = lcreate ();
        assert (lck != SYSERR,"Test 8 FAILED\n");

        rd1 = create(reader8, 2000, 20, "reader8", 3, 'A', lck, 30);
        wr1 = create(writer8, 2000, 20, "writer8", 3, 'B', lck, 30);
        wr2 = create(writer8, 2000, 20, "writer8", 3, 'C', lck, 25);
    
        //kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(wr2);
        sleep (1);

        //kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume (rd1);
        sleep(3);
        resume (wr1);

        sleep (10);
        ldelete (lck);
        kill(rd1);kill(wr2);kill(wr1);
        kprintf("Output is %s\n",output8);
        assert(strncmp(output8,"CCAABB",6)==0,"Test 8 FAILED\n");
        kprintf ("Test 8 PASSED\n");
        totalpassed += 1;

}
/*----------------------------------Test 8---------------------------*/
char output9[6];
int count9;
void reader9 (char i, int lck, int lprio)
{
        int     ret;

        //kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output9[count9++]=i;
        //kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (3);
        //kprintf ("  %s: to release lock\n", msg);
        output9[count9++]=i;
        releaseall (1, lck);
        
}

void writer9 (char i, int lck, int lprio)
{
        //kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output9[count9++]=i;
        //kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (4);
        //kprintf ("  %s: to release lock\n", msg);
        output9[count9++]=i;
        releaseall (1, lck);
        
}

void test9 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1,wr2;

        count9 = 0;
        kprintf("\nTest 9: wait on locks with equal priority, waiting time diff < 1s. Writer should be given preference. Expected order of "
        "lock acquisition is: writer C, writer B, reader A\n");
        lck  = lcreate ();
        assert (lck != SYSERR,"Test 9 FAILED\n");

        rd1 = create(reader9, 2000, 20, "reader9", 3, 'A', lck, 30);
        wr1 = create(writer9, 2000, 20, "writer9", 3, 'B', lck, 30);
        wr2 = create(writer9, 2000, 20, "writer9", 3, 'C', lck, 25);
    
        //kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(wr2);
        sleep (1);

        //kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume (rd1);
        resume (wr1);


        sleep (10);
        ldelete (lck);
        kill(rd1);kill(rd2);kill(wr1);
        kprintf("Output is %s\n",output9);
        assert(strncmp(output9,"CCBBAA",6)==0,"Test 9 FAILED\n");
        kprintf ("Test 9 PASSED\n");
        totalpassed += 1;

}
/*----------------------------------Test 10---------------------------*/
void reader10 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer10 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test10 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 10: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR,"Test 10 FAILED");

        rd1 = create(reader10, 2000, 25, "reader10", 2, "reader A", lck);
        rd2 = create(reader10, 2000, 30, "reader10", 2, "reader B", lck);
        wr1 = create(writer10, 2000, 20, "writer10", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
        kprintf("PIO = %d for process %d\n", getprio(wr1), wr1);
        assert (getprio(wr1) == 25, "Test 10 FAILED\n");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
        sleep (1);
        assert (getprio(wr1) == 30,"Test 10 FAILED\n");
    
        kprintf("-kill reader B, then sleep 1s\n");
        kill (rd2);
        sleep (1);
        assert (getprio(wr1) == 25,"Test 10 FAILED\n");

        kprintf("-kill reader A, then sleep 1s\n");
        kill (rd1);
        sleep(1);
        assert(getprio(wr1) == 20,"Test 10 FAILED\n");

        sleep (8);
        ldelete(lck);
        kprintf ("Test 10 PASSED\n");
        totalpassed += 1;
}

/*----------------------------------Test 11---------------------------*/
void reader11 (char *msg, int lck)
{
        int     ret;

        //kprintf ("  %s: to acquire lock, will block\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        //kprintf ("  %s: acquired lock\n", msg);
        //kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer11a (char *msg, int lck1, int lck2)
{
        //kprintf ("  %s: to acquire lock1\n", msg);
        lock (lck1, WRITE, DEFAULT_LOCK_PRIO);
        //kprintf ("  %s: acquired lock1\n", msg);

        //kprintf ("  %s: to acquire lock2, will block\n", msg);
        lock (lck2, WRITE, DEFAULT_LOCK_PRIO);
        //kprintf ("  %s: acquired lock2\n", msg);
        //kprintf ("  %s: to release lock\n", msg);
        releaseall (2, lck1, lck2);
}

void writer11b (char *msg, int lck)
{
        //kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        //kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (5);
        //kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test11 ()
{
        int     lck1, lck2;
        int     rd1;
        int     wr1, wr2;

        kprintf("\nTest 11: test the transitivity of priority inheritence\n");
        //kprintf("write A <25> holds lock 1, waiting for lock 2 which was acquired \n");
        //kprintf("by write B <20>. read C <30> tries to acquire lock 2 and blocked.\n");
        lck1  = lcreate ();
        assert (lck1 != SYSERR,"Test 11 FAILED\n");
    
        lck2  = lcreate ();
        assert (lck2 != SYSERR,"Test 11 FAILED\n");

        rd1 = create(reader11,  2000, 30, "reader9", 2, "reader C", lck1);
        wr1 = create(writer11a, 2000, 25, "writer9", 3, "writer A", lck1, lck2);
        wr2 = create(writer11b, 2000, 20, "writer9", 2, "writer B", lck2);

        //kprintf("-start writer B. lock2 granted to write B <20>\n");
        resume(wr2);
        sleep (1);
    
        //kprintf("-start writer A. write A is granted lock1, and block on lock2<25>\n");
        resume (wr1);
        sleep (1);
    
        assert (getprio (wr2) == 25,"Test 11 FAILED\n");
    
        //kprintf("-start reader C, reader C<30> will block on lock1\n");
        resume (rd1);
        sleep (1);
        assert (getprio (wr2) == 30,"Test 11 FAILED\n");
        assert (getprio (wr1) == 30,"Test 11 FAILED\n");

        sleep (5);
        ldelete(lck1);
        ldelete(lck2);
        kprintf ("Test 11 PASSED\n");
        totalpassed += 1;
}
char output12[4];
int count12;
/* ----------------- test12 --------------------------- */
void reader12(char i, int lck, int lprio) {
    int rc;
    //kprintf ("  %s: to acquire lock\n", msg);
    rc = lock(lck, READ, lprio);
    output12[count12++]=i;
    //kprintf ("  %s: acquired lock, sleep 3s\n", msg);
    sleep (5);
    //kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
}
void reader12a(char i) {
    while(1){}
}
void writer12b (char i, int lck, int lprio) 
{
        //kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        output12[count12++]=i;
        //kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (5);
        //kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}


void test12()
{       
    int lck;
    int rd1, rd2, rd3, wr1;
    
    count12 = 0;
    kprintf("\nTest 12: starvation avoidance.\n"
    "Expected order of lock acquisition is: A,B,D\n");
    lck  = lcreate ();


    
    rd1 = create(reader12, 2000, 10, "reader", 3, 'A', lck, 20);
    rd2 = create(reader12, 2000, 10, "reader", 3, 'B', lck, 20);
    rd3 = create(reader12a, 2000, 19, "reader",1,'C');
    wr1 = create(writer12b, 2000, 90, "reader", 3, 'D', lck, 20);


    //kprintf("-start RW30_1. Sleep 2s\n");
    resume(rd1);sleep10(5);resume(rd2);
    sleep(1);
    resume(wr1);
    sleep(1);
    resume(rd3);
    
    sleep (10);
    kill(wr1),kill(rd3);
    output12[count12++]='F';
    kprintf("Output is %s\n",output12);
    assert(strncmp(output12,"ABD",3)==0,"Test 12 FAILED\n");
    kprintf ("Test 12 PASSED\n");
    totalpassed += 1;
}

void test0()
{
    kprintf("[%d] Sleeping for 1 second\n", currpid);
    sleep10 (15);
    kprintf("[%d] Wake up...\n", currpid);
}

void sleep00(int n)
{
    sleep(n);
}

void test00()
{
    int p1 = create(sleep00, 2000, 10, "reader", 3, 1);
    int p2 = create(sleep00, 2000, 10, "reader", 3, 1);
    resume(p1);
    resume(p2);
    sleep10(15);
}

int main( )
{
    test00();
    test0();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    test11();
    test12();
    kprintf("\n--------------SUMMARY------------\n");
    kprintf("PASSED: %d, FAILED: %d, TOTAL: %d\n",totalpassed,totalfailed,totalpassed+totalfailed);
    kprintf("Score = %d\n",60-totalfailed*5);

    /* the hook to shutdown QEMU for process-like execution of XINU */
    shutdown();
}




