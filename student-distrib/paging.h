#ifndef _PAGING_H
#define _PAGING_H

#include "lib.h"
#define VIDEO 0xB8000

#define PAGE_SIZE 1024
#define PAGE_ALIGN PAGE_SIZE * 4

#define DIR_IDX_SHIFT 22
#define TABLE_IDX_SHIFT 12
#define CLEAR_DIR_IDX 0x003FFFFF

// just needs to be greater than 128MB
#define VIRT_VID_TERM1	0x10000000
#define VIRT_VID_TERM2	0x10001000  // 4kb offset
#define VIRT_VID_TERM3	0x10002000

// physical back up memory locations
#define BACKUP_VID1 0xB9000
#define BACKUP_VID2 0xBA000
#define BACKUP_VID3 0xBB000

#define KERNEL_MEM    0x00400000  // start of kernel memory (physical and virtual)
#define RW_SET_ONLY   0x00000002  // Set bit 1, enables r/w
#define RW_P_SET      (RW_SET_ONLY | 0x00000001) // Set bit 0, enables present bit
#define RW_P_SIZE_SET (RW_P_SET    | 0x00000080) // Set bit 7, enables larger page size
#define PROCESS_SET   0x00000087 // Set size, user, r/w, present
#define USER_MASK     0x7		//set 4kb size, user, r/w, present
#define USER_VIDEO_ 	 (VIDEO | USER_MASK) //set user video memory page (4kb)
// set user video mem for back up 1, 2, and 3
#define USER_BACK1	 (BACKUP_VID1 | USER_MASK)
#define USER_BACK2	 (BACKUP_VID2 | USER_MASK)
#define USER_BACK3	 (BACKUP_VID3 | USER_MASK)

#define USER_SPACE    0x00800000
#define PROCESS_SIZE_ 0x00400000
#define PROCESS_IDX   32

/* page directory */
uint32_t page_directory[PAGE_SIZE] __attribute__((aligned(PAGE_ALIGN)));

/* page table for video memory */
uint32_t page_table[PAGE_SIZE] __attribute__((aligned(PAGE_ALIGN)));
uint32_t user_page_table[PAGE_SIZE] __attribute__((aligned(PAGE_ALIGN)));
uint8_t vidMemPageTable[PAGE_SIZE] __attribute__((aligned(PAGE_ALIGN)));
/* initializes paging */
void paging_init();

/* helper function to enable paging (in-line-assembly) */
void enablePaging();

/* allocate page for a new process */
void add_process(uint32_t process_id);

/* allocate video memory page for user */
void map_virt_to_phys(uint32_t virtual_address, uint32_t PHYS);
void repage(uint32_t virtual_addr, uint32_t physical_addr);
extern void PageTableToPage(uint32_t virtualAddr, uint32_t physicalAddr, uint32_t page);

/* clear the TLB */
void set_up_map(uint32_t virtualAddr, uint32_t physicalAddr);
void flush_tlb();

#endif
