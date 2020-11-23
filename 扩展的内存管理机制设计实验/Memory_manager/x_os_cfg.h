#define CFGFILE                                    		/* define this file for config  */        

#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "semLib.h"
#include "taskLib.h"
#include "sysLib.h"						/* include the header file	*/

#ifdef   OS_GLOBALS
#define  OS_EXT
#else
#define  OS_EXT  extern
#endif								/* global variable define	*/


#define X_OS_MEM_EN                     1u                  	/* enable memory management	*/
#define X_OS_ARG_CHK_EN                 1u                 	/* enable arg check 		*/

/*
**********************************************************************************
*
*				partition cfg
*
*Description : this is the partition config , we can use this as an interface to 
*		configure the number of memory pools,block size and block number
*
*Arguments   : none
*
*Returns     : none
*
*Notes       : X_MCBNMB could not over 110 because the higher num is used for cfg
*		and at least 2, [2,110)
*
**********************************************************************************
*/
#define X_MCBNMB                        3u                  	/*memory control block num it could not over 110 because the higher num is used for cfg*/
#define X_BLOCKSIZE			{40u,128u,256u}		/*memory block table		*/
#define X_BLOCKNUM			{5u,10u,10u}		/*memory block num table	*/


/* there are many modes ,and the mode allocated from the mem pool is [0,X_MCBNMB) */
/* if the mem pool could not afford the size we want,it will automatically convert to Heap mode */
#define X_MEM_MOD_HEAP                  111u                 	/* Heap mode			*/
#define X_MEM_SIZE_ZERO			112u			/* malloc size is zero		*/
#define X_MEM_MOD_RESERVE		113u			/* mode reserve tmp             */

/* sys bool variable */ 
#define X_OS_TRUE                       0u                 	/* for true			*/
#define X_OS_FALSE                      1u                	/* for false			*/

/* err code for create partition */
#define X_OS_ERR_DIV_NONE               0u           		/* no err			*/       
#define X_OS_ERR_DIV_MEM_INVALID_ADDR   1u                  	/* err for invalid addr arg	*/
#define X_OS_ERR_DIV_MEM_INVALID_BLKS   2u                	/* err for blks num less 2	*/
#define X_OS_ERR_DIV_MEM_INVALID_SIZE   3u                  	/* err for blks size too small	*/
#define X_OS_ERR_DIV_MEM_INVALID_PART   4u                 	/* MCB left zero		*/


#define PutErr
#define X_OS_PUT_ERR_NONE               0u 			/* no err			*/                 
#define X_OS_ERR_PUT_NO_MODE            1u            		/* mode choose failed		*/     
#define X_OS_ERR_PUT_INVALID_MODE       2u                 	/* mode invalid			*/
#define X_OS_ERR_PUT_INVALID_PBLK       3u                 	/* pblk invalid			*/
#define X_OS_ERR_PUT_MEM_FULL           4u                	/* partition full		*/


#define GetErr
#define X_OS_GET_ERR_NONE               0u      		/* no err			*/            
#define X_OS_ERR_GET_NO_MODE            1u                	/* mode choose failed		*/ 
#define X_OS_ERR_GET_INVALID_MODE       2u                  	/* MODE invalid			*/
#define X_OS_ERR_GET_MALLOC_FAIL        3u                 	/* malloc failed		*/
#define X_OS_ERR_GET_NO_FREE_BLKS       4u                 	/* no free blk in partition	*/
#define X_OS_VERY_LARGE_TOP		1000u			/* each block size should not over it */
typedef unsigned int X_INT32U;
typedef unsigned char X_INT8U;

#if X_OS_MEM_EN > 0

typedef struct{
    void   *OSMemAddr;                                     	/* partition begin addr		*/
    void   *OSMemEndAddr;                                  	/* partition end addr		*/
    void   *OSMemFreeList;                                	/* blks freeList		*/
    X_INT32U  OSMemBlkSize;                               	/* blks size			*/
    X_INT32U  OSMemNBlks;                                 	/* blks num			*/
    X_INT32U  OSMemNFree;                                  	/* free blks num		*/
}X_OS_MEM;

/***************************************INIT BLOCK TABLE START***********************************************/
#ifndef MAIN_SOURCE
OS_EXT X_INT32U X_BLOCKSIZE_TABLE[X_MCBNMB];			/* define blocksizetable for each partition */
OS_EXT X_INT32U X_BLOCKNUM_TABLE[X_MCBNMB];			/* define blocknumtable for each partition  */
#else
OS_EXT X_INT32U X_BLOCKSIZE_TABLE[X_MCBNMB] = X_BLOCKSIZE;	/* define blocksizetable for each partition */
OS_EXT X_INT32U X_BLOCKNUM_TABLE[X_MCBNMB] = X_BLOCKNUM;	/* define blocknumtable for each partition  */
#endif
/***************************************         END          ***********************************************/

/***************************************   CORE G.V. DEFINE   ***********************************************/
OS_EXT void *pmalloc_table[X_MCBNMB];                          	/* pointer of partition after malloc from Heap */
OS_EXT X_OS_MEM *gpmem_table[X_MCBNMB];                         /* pointer of MCB		*/
OS_EXT SEM_ID semMforPTable[X_MCBNMB];                   	/* sem use for all partition 	*/                
/***************************************         END          ***********************************************/

OS_EXT X_OS_MEM *X_MemFreeList;					/* MCB freeList			*/                            
OS_EXT X_OS_MEM X_MemControlBlock[X_MCBNMB];      		/* MCB array			*/         
OS_EXT SEM_ID semMforgv;					/* sem for g.v.			*/                                   

OS_EXT X_INT8U HeapCount;					/* malloc count from Heap	*/

#endif


/*************************************** Function declaration ***********************************************/
X_INT8U gvInit();
void MemBlockInit();
X_OS_MEM  *MemCreate (void   *addr,X_INT32U  nblks,X_INT32U  blksize,X_INT8U  *perr);
X_INT8U MemPartCreate(X_INT8U perr_table[X_MCBNMB]);
X_INT8U MemGetCh(X_INT32U size);
void *MemGetFromPartition(X_INT8U mode,X_INT8U *perr);
X_INT8U MemPutCh(void *pblk);
X_INT8U MemPutToPartition(X_INT8U mode,void *pblk);
void MemInit();
void *OSMemAlloc(X_INT32U size,X_INT8U *perr);
X_INT8U OSMemFree(void *pblk);
void show();