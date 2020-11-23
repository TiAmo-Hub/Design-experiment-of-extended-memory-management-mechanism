#define OS_GLOBALS
#define MAIN_SOURCE
#include "x_os_cfg.h" /* declaration of g.v. and #define */


/*
**********************************************************************************
*
*				MemInit
*
*Description : this function is called before the sys run,used for sys init
*
*Arguments   : none
*
*Returns     : none
*
*Notes       : none
*
**********************************************************************************
*/
void MemInit(){
    X_INT8U i=0u;
    X_INT8U err=0u;
    X_INT8U perr_table[X_MCBNMB]={0};
    err=gvInit();					/* g.v. init		*/                          
    if(err!=0)
    {
        printf("gvbInit gv failed \n");
        return ;
    }
    MemBlockInit();					/* MCB init		*/                   
    err=0u;
    err=MemPartCreate(perr_table);			/* partition init	*/                    
    if(err!=0u){
        printf("mem div init failed \n");
        for(i;i<X_MCBNMB;i++){
            printf("init mem %d for perr -> %d \n",i,perr_table[i]);
        }
        return ;
    }
}

/*
**********************************************************************************
*
*				OSMemAlloc
*
*Description : this function is used to get the mem from partitions or 
*		Heap,the mode is dicided by the size passed by user
*
*Arguments   : X_INT32U size, the size we want to get
*	       X_INT8U *perr, the err code 
*
*Returns     : (void *)res pointer to the mem alloc,if an err occured,it will 
*		return (void *)0  
*
*Notes       : none
*
**********************************************************************************
*/
void *OSMemAlloc(X_INT32U size,X_INT8U *perr){
    X_INT8U mode=X_MEM_MOD_RESERVE;			/* at the beginning.assign the mode to X_MEM_MOD_RESERVE means no mode */
    X_INT8U err=0u;
    void *res=(void*)0;
    mode = MemGetCh(size);
    if(mode==X_MEM_SIZE_ZERO){				/* if the size passed is 0 */
        *perr=X_OS_ERR_GET_NO_MODE;
        return (void *)0;
    }
    else if(mode==X_MEM_MOD_HEAP){			/* the mode is Heap,it can be caused by many reason:1.the size is too lage 2. all of the pools is full*/
        res=malloc(size);
        if(res==(void*)0)*perr=X_OS_ERR_GET_MALLOC_FAIL;
        else{
             HeapCount++;				/* record the Heap used count */
             *perr=X_OS_GET_ERR_NONE;
        }
        return res;
    }
    else if(mode>=0 && mode<X_MCBNMB){			/* if it is a normal case, we will choose a mode between [0,X_MCBNUM) */
        res=MemGetFromPartition(mode,&err);
        if(res==(void*)0)*perr=err;
        else *perr=X_OS_GET_ERR_NONE;
        return res;
    }else if(mode == X_MEM_MOD_RESERVE){		/* if the mode is not being changed , an err occured */
        *perr=X_OS_ERR_GET_NO_MODE;
        return (void *)0;
    }
}

/*
**********************************************************************************
*
*				OSMemFree
*
*Description : this function is called to free the mem we got before
*
*Arguments   : void *pblk pointer to the blk we want to free
*
*Returns     : X_INT8U err code
*
*Notes       : none
*
**********************************************************************************
*/
X_INT8U OSMemFree(void *pblk){
    X_INT8U mode=X_MEM_MOD_RESERVE;			/* at the beginning.assign the mode to X_MEM_MOD_RESERVE means no mode */
    X_INT8U res=0u;
    mode = MemPutCh(pblk);				/* choose the mode */
    if(mode==X_MEM_MOD_RESERVE)return X_OS_ERR_PUT_NO_MODE; /* if the mode is not being changed , an err occured */
    else if(mode>=0 && mode<X_MCBNMB){
        res=MemPutToPartition(mode,pblk);
        return res;
    }							/* if it is a normal case, we will choose a mode between [0,X_MCBNUM) */
    else if (mode==X_MEM_MOD_HEAP){			/* if the mem we got is from Heap */
        free(pblk);
        HeapCount--;					/* record the Heap used count */
        return X_OS_PUT_ERR_NONE;
    }
}