#ifndef __PAGING_H
#define __PAGING_H
#include "types.h"
#ifndef ASM

/* initialize paging by setting up page directory and page table */
extern void page_init();
/* map virtual address to physical address */
extern void map_4MB_page(uint32_t pid);
/* map video memory address into user space */
extern void map_video_mem(uint32_t addr);
/* map page table entry to updated video buffer */
extern void map_video_page(uint32_t addr);
/* map a 4KB page in page table */
extern void map_4KB_page(uint32_t physical);

#endif
#endif
