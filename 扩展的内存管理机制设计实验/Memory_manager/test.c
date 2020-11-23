#include "x_os_cfg.h"

#define STACK_SIZE  2000
     
int    tid1;                 
/*int    tid2;
int    tid3;                 
int    tid4;
int    tid5;                 
int    tid6;*/
void progStart(void);
void   taskTem(int id,unsigned int size,int timedelay);
void   taskTem2(int id,unsigned int size,int timedelay);
void   progStop(void);

void progStart(void)
{
      MemInit();
      printf("\n\nMemory Init Table ->");
      show();
      /* The test environment is as follows: 
             Test 1: each memory pool has a memory block request that
	 matches its memory range. 
	     Test 2: Tests if the amount of memory requested exceeds the 
	 block number of the memory pool, generating a warning, and then 
	 requests memory from other memory pools. 
	     Test 3: Free memory that exceeds the number of blocks in the
	 memory pool 
      */
      /* Test 1
      tid1    = taskSpawn ("task1", 230, 0, STACK_SIZE,(FUNCPTR)taskTem,1, 30, 1, 0, 0, 0, 0, 0, 0, 0);	
      tid2    = taskSpawn ("task2", 220, 0, STACK_SIZE,(FUNCPTR)taskTem,2, 128, 2, 0, 0, 0, 0, 0, 0, 0);
      tid3    = taskSpawn ("task3", 210, 0, STACK_SIZE,(FUNCPTR)taskTem,3, 200, 2, 0, 0, 0, 0, 0, 0, 0);
      tid4    = taskSpawn ("task4", 200, 0, STACK_SIZE,(FUNCPTR)taskTem,4, 500, 2, 0, 0, 0, 0, 0, 0, 0); 
      */
      /* Test 2 
      tid1    = taskSpawn ("task1", 230, 0, STACK_SIZE,(FUNCPTR)taskTem,1, 30, 200, 0, 0, 0, 0, 0, 0, 0);	
      tid2    = taskSpawn ("task2", 220, 0, STACK_SIZE,(FUNCPTR)taskTem,2, 30,200, 0, 0, 0, 0, 0, 0, 0);
      tid3    = taskSpawn ("task3", 210, 0, STACK_SIZE,(FUNCPTR)taskTem,3, 30, 200, 0, 0, 0, 0, 0, 0, 0);
      tid4    = taskSpawn ("task4", 200, 0, STACK_SIZE,(FUNCPTR)taskTem,4, 30, 200, 0, 0, 0, 0, 0, 0, 0);
      tid5    = taskSpawn ("task5", 190, 0, STACK_SIZE,(FUNCPTR)taskTem,3, 30, 200, 0, 0, 0, 0, 0, 0, 0);
      tid6    = taskSpawn ("task6", 180, 0, STACK_SIZE,(FUNCPTR)taskTem,4, 30, 200, 0, 0, 0, 0, 0, 0, 0);
      */
      tid1    = taskSpawn ("task1", 230, 0, STACK_SIZE,(FUNCPTR)taskTem2,1, 30, 200, 0, 0, 0, 0, 0, 0, 0);	
      return;
}

void taskTem2(int id,unsigned int size,int timedelay)
{ 
      
	X_INT8U I=0u;
        void *xyy=(void *)0;
	while(1){     
              X_INT8U perr=0u;
              if(I==0){
                 printf("\nTask %d Malloc ->",id);
                 xyy=OSMemAlloc(size,&perr);		/* alloc mem from sys */
                 show();
                 taskDelay(timedelay);
	      }	
              printf("\nTask %d Free ->",id);
              OSMemFree(xyy);				/* free mem to sys */
              show();
              taskDelay(timedelay);
              if(I==0){I=1u;}
              
      }
      return;
}

void taskTem(int id,unsigned int size,int timedelay)
{    
	while(1){     
              X_INT8U perr=0u;
              void *xyy=(void *)0;
              printf("\nTask %d Malloc ->",id);
              xyy=OSMemAlloc(size,&perr);		/* alloc mem from sys */
              show();
              taskDelay(timedelay);
              printf("\nTask %d Free ->",id);
              OSMemFree(xyy);				/* free mem to sys */
              show();
              taskDelay(timedelay);
              
      }
      return;
}

void progStop(void)
{

      taskDelete(tid1);
     /* taskDelete(tid2);
      taskDelete(tid3);
      taskDelete(tid4);
      taskDelete(tid5);
      taskDelete(tid6);*/
      printf("The End\n");

      return;
}