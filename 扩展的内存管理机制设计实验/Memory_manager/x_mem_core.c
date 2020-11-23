#include "x_os_cfg.h" /* declaration for g.v. and #define */

#if X_OS_MEM_EN >=1u

/*
**********************************************************************************
*
*				OS_MemClr
*
*Description : this function is called to clear the mem by Byte
*
*Arguments   : X_INT8U  *pdest, the destination of mem we want to clear
*              X_INT32U  size, the range of the mem 
*
*Returns     : none
*
*Notes       : none
*
**********************************************************************************
*/
void  OS_MemClr (X_INT8U  *pdest,
                 X_INT32U  size)
{
    while (size > 0u) {
        *pdest++ = (X_INT8U)0;			/* clear the mem by Byte	*/
        size--;
    }
}

/*
**********************************************************************************
*
*				gvInit
*
*Description : this function is called to init the g.v. and malloc the mem of 
*               partitions from Heap
*
*Arguments   : none
*
*Returns     : X_INT8U err code
*		       X_OS_FALSE : false
*		       X_OS_TRUE  : true
*
*Notes       : none
*
**********************************************************************************
*/
X_INT8U gvInit(){
    X_INT8U i=0u;
    X_MemFreeList        = (void *)0;
    HeapCount            = 0u;      			/* init HeapCount for 0u */                                  
    semMforgv            = semMCreate(SEM_Q_PRIORITY);  /* init the sem for g.v. ,the sem is Mutex */    
    printf("\nthe semMforgv id is -> %x",(X_INT32U)semMforgv);  
    for(i;i<X_MCBNMB;i++){
        gpmem_table[i]   = (void *)0;			/* assign the pointer (void *)0 for now for temporary */  
        semMforPTable[i] = semMCreate(SEM_Q_PRIORITY);	/* partitions sem create */
        printf("\nthe semMforPool %d id is -> %x",i+1,(X_INT32U)semMforPTable[i]); 
        pmalloc_table[i] = malloc(X_BLOCKSIZE_TABLE[i]*X_BLOCKNUM_TABLE[i]);/* malloc mem for each partition */
    	if( pmalloc_table[i] == (void *)0 ){
            return X_OS_FALSE;				/* if an err occur,return immediately */
        }
    }									          
    return X_OS_TRUE;					/* return the err code */
}

/*
**********************************************************************************
*
*				MemBlockInit
*
*Description : this function is called to initializes the memory control block and 
* 		links the MCB array into a linked list
*
*Arguments   : none
*
*Returns     : none
*
*Notes       : none
*
**********************************************************************************
*/
void MemBlockInit(){

#if X_MCBNMB >= 2u					/* the sys require 2 MCB at least */
    X_OS_MEM  *pmem;
    X_INT8U  i;

    OS_MemClr((X_INT8U *)&X_MemControlBlock[0], sizeof(X_MemControlBlock)); /* clear the mem by byte */
    for (i = 0u; i < (X_MCBNMB - 1u); i++) {       
        pmem                = &X_MemControlBlock[i];               
        pmem->OSMemFreeList = (void *)&X_MemControlBlock[i + 1u];  
    }							/* link the array into list */

    pmem                = &X_MemControlBlock[i];
    pmem->OSMemFreeList = (void *)0;                      
    X_MemFreeList   = &X_MemControlBlock[0];            /* assign a true value to the g.v. X_MemFreeList */          
#endif
}                                              
/*
**********************************************************************************
*
*				MemCreate
*
*Description : this function is called to initializes the memory partition and links
* 		the blcoks into a linked list
*
*Arguments   : void   *addr, the beginning addr of the partition 
*              X_INT32U  nblks, the block num we want to create in the partiton
*              X_INT32U  blksize, the block size for each block
*              X_INT8U  *perr, the err code
*
*Returns     : X_OS_MEM * , return pointer to the MCB which control the partition we 
*		just create
*
*Notes       : none
*
**********************************************************************************
*/
X_OS_MEM  *MemCreate (void   *addr,
                      X_INT32U  nblks,
                      X_INT32U  blksize,
                      X_INT8U  *perr)
{
    X_OS_MEM  *pmem;						/* pointer to the mcb */
    X_INT8U     *pblk;						/* pointer to the block */
    void     **plink;						/* the second pointer to the block */
    X_INT32U     loops;
    X_INT32U     i;


#if X_OS_ARG_CHK_EN > 0u
	 
    if (addr == (void *)0) {                     
        *perr = X_OS_ERR_DIV_MEM_INVALID_ADDR;
        return ((X_OS_MEM *)0);
    }
   
    if (nblks < 2u) {                                
        *perr = X_OS_ERR_DIV_MEM_INVALID_BLKS;
        return ((X_OS_MEM *)0);
    }
    
    if (blksize < sizeof(void *)) {                  
        *perr = X_OS_ERR_DIV_MEM_INVALID_SIZE;
        return ((X_OS_MEM *)0);
    }
#endif								/* check the arg we passed */
    semTake(semMforgv, WAIT_FOREVER);				/* we must take the sem in order to get the g.v. */
    pmem = X_MemFreeList;                            
    if (X_MemFreeList != (X_OS_MEM *)0) {               
        X_MemFreeList = (X_OS_MEM *)X_MemFreeList->OSMemFreeList;
    }								/* we get a free MCB from the g.v. X_MemFreeList */
    semGive(semMforgv);						/* give the sem */
    if (pmem == (X_OS_MEM *)0) {                       
        *perr = X_OS_ERR_DIV_MEM_INVALID_PART;
        return ((X_OS_MEM *)0);
    }								/* check the mcb pointer to prevent the pointer is null */
   
    plink = (void **)addr;                            
    pblk  = (X_INT8U *)addr;
    loops  = nblks - 1u;
    for (i = 0u; i < loops; i++) {
       
        pblk +=  blksize;                             
       
       *plink = (void  *)pblk;                        
       
        plink = (void **)pblk;                       
    }								/* link the blocks into a link list */
    pmem->OSMemAddr     = addr;  
    pmem->OSMemEndAddr  = (void *)plink + blksize;          
    pmem->OSMemFreeList = addr;                       
    pmem->OSMemNFree    = nblks;                      
    pmem->OSMemNBlks    = nblks;
    pmem->OSMemBlkSize  = blksize;                    		/* fill in the corresponding value in the MCB */
    

    *plink              = (void *)0;        			/* the last block point to null */
    
    *perr               = X_OS_ERR_DIV_NONE;

    return (pmem);
}
/*
**********************************************************************************
*
*				MemPartCreate
*
*Description : this function is called to initializes the pointer array to the MCB
*
*Arguments   : X_INT8U perr_table[X_MCBNMB] ,err code
*
*Returns     : X_INT8U , the status of memory create
*
*Notes       : none
*
**********************************************************************************
*/
X_INT8U MemPartCreate(X_INT8U perr_table[X_MCBNMB]){
    X_INT8U i=0u;
    X_INT8U flag=0u;
    for(i;i<X_MCBNMB;i++){
    	gpmem_table[i] = MemCreate((void *)pmalloc_table[i],X_BLOCKNUM_TABLE[i],X_BLOCKSIZE_TABLE[i],&perr_table[i]);
    	if(perr_table[i]!=X_OS_ERR_DIV_NONE)flag=1u;
    }							/* we create the partition one by one */
    if(flag==1u)return X_OS_FALSE;
    return X_OS_TRUE;					/* return the err code */
}
/*
**********************************************************************************
*
*				MemGetCh
*
*Description : this function is called to get the mode if we want to get a mem block
*		from the sys
*
*Arguments   : X_INT32U size, the size we want get ,it decide the mode which we choose
*
*Returns     : X_INT8U the mode we choose
*
*Notes       : in this function ,we set the local variable temSize to X_OS_VERY_LARGE_TOP,
*		strictly speaking , this is not rigorous, we can only assume that
*		all the size of the memory block is less than this value,
*		so that the best adaptive strategy can be adopted to obtain the memory block.
*
**********************************************************************************
*/
X_INT8U MemGetCh(X_INT32U size){
    X_INT8U i=0u;
    X_INT32U temSize=X_OS_VERY_LARGE_TOP;	/* record the size of the best fit for now */			
    X_INT8U temCH   =X_MEM_MOD_HEAP;            /* record the choose mode , default Heap   */
    if(size == 0u)return X_MEM_SIZE_ZERO;
    for(i;i<X_MCBNMB;i++)
    {
        if(size > 0u && size <= X_BLOCKSIZE_TABLE[i])
	{
             if(((X_OS_MEM *)gpmem_table[i])->OSMemNFree > 0u){
   		 if(X_BLOCKSIZE_TABLE[i]<temSize){              
		    temCH = i;			
      		    temSize =X_BLOCKSIZE_TABLE[i];
                 }      			 /* if we can get a better size to get the block,then choose it and change the tem mode	*/
	     }
	}
    }
    return temCH;

}
/*
**********************************************************************************
*
*				MemGetFromPartition
*
*Description : this function is called to get the real mem block from the partition
*
*Arguments   : X_INT8U mode,the mode we choose before
*		X_INT8U *perr ,err code
*
*Returns     : void * ,the pointer of the mem block we get from the partition
*
*Notes       : none
*
**********************************************************************************
*/
void *MemGetFromPartition(X_INT8U mode,X_INT8U *perr){
    
    void      *pblk;
  
#if OS_ARG_CHK_EN > 0u
    if (mode >=X_MCBNMB && mode !=X_MEM_MOD_HEAP) {                  
        return X_OS_ERR_GET_INVALID_MODE;
    }
#endif								/* check the arg we passed */

    X_OS_MEM *pmem = gpmem_table[mode];				/* get the pointer of the MCB */
    semTake(semMforPTable[mode],WAIT_FOREVER);			/* get the partition sem */
    
    if (pmem->OSMemNFree > 0u) {                      		/* check the block num */
        pblk                = pmem->OSMemFreeList;    
       
        pmem->OSMemFreeList = *(void **)pblk;        
       
        pmem->OSMemNFree--;                           
        semGive(semMforPTable[mode]);     
        *perr = X_OS_GET_ERR_NONE;              
        return (pblk);                               
    }								/* pick down a block from the block list and return the pointer */
    semGive(semMforPTable[mode]); 
    *perr = X_OS_ERR_GET_NO_FREE_BLKS;     
    return ((void *)0);                              
}

/*
**********************************************************************************
*
*				MemPutCh
*
*Description :  this function is called to get the mode if we want to put a mem block
*		to the sys
*
*Arguments   : void *pblk, the pointer to the block which we want to return
*
*Returns     : X_INT8U, the mode we choose
*
*Notes       : the idea behind this pattern selection is to compare the passed 
*		addr to the first and the last addr of each memory pool,and if 
*		the arg in the memory pool we are comparing now,we will place 
*		it into the memory pool.
*
**********************************************************************************
*/
X_INT8U MemPutCh(void *pblk){
    X_INT8U i=0u;
    X_INT32U tmp=(X_INT32U)pblk;
    X_INT32U begin,end;
    for(i;i<X_MCBNMB;i++){
       begin=(X_INT32U)gpmem_table[i]->OSMemAddr;
       end=(X_INT32U)gpmem_table[i]->OSMemEndAddr;
       if(tmp>=begin && tmp < end){
          return i;
       }						/* compare the addr of each memory pool */
    }
    return X_MEM_MOD_HEAP;
}
/*
**********************************************************************************
*
*				MemPutToPartition
*
*Description : this function is called to return the block to the partition
*
*Arguments   : X_INT8U mode, the mode we choose before
*		void *pblk, the pointer of the block we want to return
*
*Returns     : X_INT8U,the err code
*
*Notes       : none
*
**********************************************************************************
*/
X_INT8U MemPutToPartition(X_INT8U mode,void *pblk){


#if OS_ARG_CHK_EN > 0u
   if (mode >=X_MCBNMB && mode !=X_MEM_MOD_HEAP) {                  
        return X_OS_ERR_PUT_INVALID_MODE;
    }
    if (pblk == (void *)0) {                    
        return (X_OS_ERR_PUT_INVALID_PBLK);
    }
#endif							/* check the arg we passed in */
	
    X_OS_MEM *pmem = gpmem_table[mode];			/* get the MCB pointer */
    semTake(semMforPTable[mode], WAIT_FOREVER);		/* get the sem for the partition */
    
    if (pmem->OSMemNFree >= pmem->OSMemNBlks) {  
        semGive(semMforPTable[mode]);
        return (X_OS_ERR_PUT_MEM_FULL);
    }							/* if the memory pool is full ,an error occured */
   
    *(void **)pblk      = pmem->OSMemFreeList;   
    pmem->OSMemFreeList = pblk;
    pmem->OSMemNFree++;                         
    semGive(semMforPTable[mode]);			/* return the block successfully */
    return (X_OS_PUT_ERR_NONE);                        
}

/*
**********************************************************************************
*
*					show
*
*Description : this function is called to spy on each memory pool status
*
*Arguments   : none
*
*Returns     : none
*
*Notes       : none
*
**********************************************************************************
*/
void show(){
    X_INT8U i=0u;
    printf("\n-------------------------------------------------\n");
    printf("PARTID  MEMALL  BLKNUM  BLKSIZ  FREBLK  USEBLK\n");
    for(i;i<X_MCBNMB;i++){
        printf("%6d  %6d  %6d  %6d  %6d  %6d \n",i+1,X_BLOCKSIZE_TABLE[i]*X_BLOCKNUM_TABLE[i],gpmem_table[i]->OSMemNBlks,
						X_BLOCKSIZE_TABLE[i],gpmem_table[i]->OSMemNFree,gpmem_table[i]->OSMemNBlks-gpmem_table[i]->OSMemNFree);
    }
    printf("-------------------------------------------------\n");
    printf("HeapCount  %d\n",HeapCount);
    printf("-------------------------------------------------\n");
}
#endif