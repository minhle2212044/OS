/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* TODO update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */

  struct page_table_t *page_table = proc->page_table;
    for (int i = 0; i < page_table->size; i++) {
      struct trans_table_t *trans_table = page_table->table[i].next_lv;
      if (trans_table != NULL) {
        /* Iterate over all translation tables of the page table */
        for (int j = 0; j < trans_table->size; j++){
          /* Update TLB cache with frame number information */
          int pgnum = trans_table->table[j].v_index;
          int frame_number = trans_table->table[j].p_index;
          tlb_cache_write(proc->tlb, proc->pid, pgnum, frame_number);
        }
      }
    }
  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct * mp)
{
  /* TODO flush tlb cached*/

  if (mp == NULL || proc == NULL) {
    return -1; // Invalid arguments
  }

  // Flush TLB cache by resetting its contents
  int max_cache_size = mp->maxsz / sizeof(BYTE);
  for (int i = 0; i < max_cache_size; i++) {
    tlb_cache_write(proc->tlb, proc->pid, i, 0); // Invalidate TLB cache entry
  }
  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr, val;
  if(proc==NULL){
  	printf("Error: Proc is NULL\n");
  	return -1;
  }
  if(proc->page_table==NULL){
  	printf("Error: Proc table is NULL\n");
  	return -1;
  }
  /* By default using vmaid = 0 */
  val = __alloc(proc, 0, reg_index, size, &addr);
  if(val<0){
  	printf("Error: alloc failed\n");
  	return -1;
  }
  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  for (int i = addr; i < addr + size; i++) {
    // Calculate the page number from the address
    int pgnum = i / PAGE_SIZE;

    // Update TLB cache with the frame number
    struct page_table_t *page_table = proc->page_table;
    struct trans_table_t *trans_table = page_table->table[pgnum / (1 << FIRST_LV_LEN)].next_lv;
    int frame_number = trans_table->table[pgnum % (1 << SECOND_LV_LEN)].p_index;
    tlb_cache_write(proc->tlb, proc->pid, pgnum, frame_number);
  }

  return val;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  __free(proc, 0, reg_index);

  /* TODO update TLB CACHED frame num of freed page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  for (int pgnum = 0; pgnum < PAGING_MAX_PGN; pgnum++) {
    BYTE value;
    if (tlb_cache_read(proc->tlb, proc->pid, pgnum, &value) == 0) {
      /* Check if the TLB entry corresponds to the freed region */
      if (value == reg_index) {
        /* Invalidate the TLB entry by writing an invalid value */
        tlb_cache_write(proc->tlb, proc->pid, pgnum, 0);
      }
    }
  }
  
  return 0;
}


/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{
  BYTE data, frmnum = -1;
	
  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/
	frmnum = tlb_cache_read(proc->tlb, proc->pid, offset / PAGE_SIZE, &data);

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n", 
	         source, offset);
  else 
    printf("TLB miss at read region=%d offset=%d\n", 
	         source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  int val = __read(proc, 0, source, offset, &data);

  destination = (uint32_t) data;

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  tlb_cache_write(proc->tlb, proc->pid, offset / PAGE_SIZE, frmnum);

  return val;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  BYTE frmnum = -1;

  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/
  tlb_cache_read(proc->tlb, proc->pid, destination, &frmnum);

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
	          destination, offset, data);
	else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
            destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  val = __write(proc, 0, destination, offset, data);

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  tlb_cache_write(proc->tlb, proc->pid, destination, frmnum);

  return val;
}

//#endif
