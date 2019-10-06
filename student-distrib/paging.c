#include "paging.h"
#include "system_calls.h"
#include "terminal.h"


#define VIDEO_MEM             0xB8000
#define VID_MEM_INDEX         0xB8
#define PAGE_DIR_SIZE         1024
#define NUM_BYTES_TOTAL       4096
#define SET_PRESENT_RW        0x3
#define BIT_MASK_UPPER_20     0xFFFFFC00
#define SET_OFFSET_BITS       0x187
#define KERNEL_ADDRESS        0x400000
#define USER_PD_IDX           32
#define _8MB                  0x800000
#define _4MB                  0x400000
#define _8KB                  0x2000
#define _4KB                  0x1000
#define PAGE_SIZE             0x80 // 0 = 4KB, 1 = 4MB
#define US_FLAG               0x04 // 0 = supervisor, 1 = user level
#define PAGE_SIZE_4MB         0x80 // 0 = 4KB, 1 = 4MB
#define US_FLAG               0x04 // 0 = supervisor, 1 = user level
#define TERM_ID_SHIFT         24 // right shift 24 bits to obtain terminal id from virtual address
#define PD_SHIFT              22 // right shift 22 bits to obtain page directory bits
#define PT_SHIFT              12 // right shift 12 bits to obtain page table bits
#define PT_MASK               0x3FF // mask to obtain page table bits
#define VID_B0                0xB9000
#define VID_B1                0xBA000
#define VID_B2                0xBB000




// initialize page directory and page tables
uint32_t page_directory[PAGE_DIR_SIZE] __attribute__((aligned(NUM_BYTES_TOTAL)));
uint32_t page_table[PAGE_DIR_SIZE] __attribute__((aligned(NUM_BYTES_TOTAL)));
uint32_t vid_page_table[PAGE_DIR_SIZE] __attribute__((aligned(NUM_BYTES_TOTAL)));

/* page_init()
 *
 * Description: initialize paging by setting up page directory and page table
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void page_init()  {
    int i;

    // initialize all entries of page_directory ad page table to 0
    for (i = 0; i < PAGE_DIR_SIZE; i++) {
        // initialize all entries to 0
        page_directory[i] = 0x0;
        page_table[i] = 0x0;
        vid_page_table[i] = 0x0;
    }
    // set page-table base address, Present, R/W in the first Page directory entry
    // set up first pde and pte for video memory
    page_directory[0] = (((uint32_t)page_table) & (BIT_MASK_UPPER_20)) + SET_PRESENT_RW;

    // set page-table base address, Present, R/W, Page size in PDE
    // set up second pde for kernel
    page_directory[1] = (KERNEL_ADDRESS) + SET_OFFSET_BITS;

    // set page base address, Present in PTE
    page_table[VID_MEM_INDEX] = (VIDEO_MEM) + SET_PRESENT_RW;


    // Enable paging
    asm volatile (
        // load CR3 with address of the page directory
        "andl $0, %%eax;"
        "movl %0, %%eax;"
        "movl %%eax, %%cr3;"

        // enable PSE (4MB pages) by setting CR4
        "movl %%cr4, %%eax;"
        "orl $0x00000010, %%eax;"
        "movl %%eax, %%cr4;"

        // set the paging and protection bits of CR0
        "movl %%cr0, %%eax;"
        "orl $0x80000000, %%eax;"
        "movl %%eax, %%cr0;"
        :
        : "r"(page_directory)     /* input */
        : "%eax"
    );
}


/* map_4MB_page(uint32_t pid)
 *
 * Description: map virtual address to physical address
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void map_4MB_page(uint32_t pid){
  // map virtual address to physical memory address
  page_directory[USER_PD_IDX] = (_8MB + (_4MB * pid)) | SET_PRESENT_RW | US_FLAG | PAGE_SIZE_4MB;

  // flush TLB
  asm volatile (
      // load CR3 with address of the page directory
      "andl $0, %%eax;"
      "movl %0, %%eax;"
      "movl %%eax, %%cr3;"
      :
      : "r"(page_directory)
      :"%eax"
    );
}


/* map_video_mem(uint32_t virtual_addr)
 *
 * Description: map video memory address into user space
 * Inputs: virtual_addr -- virtual address that needs
 *                         to be mapped to physical video memory page
 * Outputs: None
 *
 */
void map_video_mem(uint32_t virtual_addr){
  // obtain the terminal id from virtual addr
  uint32_t id = (virtual_addr << PT_SHIFT) >> TERM_ID_SHIFT;
  // disable paging first
  asm volatile (
      "movl %%cr0, %%eax;"
      "orl $0x7FFFFFFF, %%eax;"
      "movl %%eax, %%cr0;"
      :
      :
      :"%eax"
    );

  if (id == screen_terminal) {
    // mapping page directory entry to video page table and set user level priviledge
    page_directory[virtual_addr >> PD_SHIFT] = ((uint32_t)vid_page_table & BIT_MASK_UPPER_20) | SET_PRESENT_RW | US_FLAG;
    // map video page table entry to directly point to video memory and set user level priviledge
    vid_page_table[(virtual_addr >> PT_SHIFT) & PT_MASK ] =  (VIDEO_MEM) | SET_PRESENT_RW | US_FLAG;
  }
  else{
    // check terminal id to determine appropriate page directory and video page table mapping
    switch(id){
      case 0:
        page_directory[virtual_addr >> PD_SHIFT] = ((uint32_t)vid_page_table & BIT_MASK_UPPER_20) | SET_PRESENT_RW | US_FLAG;
        vid_page_table[(virtual_addr >> PT_SHIFT) & PT_MASK ] =  (VID_B0) | SET_PRESENT_RW | US_FLAG;
        break;

      case 1:
        page_directory[virtual_addr >> PD_SHIFT] = ((uint32_t)vid_page_table & BIT_MASK_UPPER_20) | SET_PRESENT_RW | US_FLAG;
        vid_page_table[(virtual_addr >> PT_SHIFT) & PT_MASK ] =  (VID_B1) | SET_PRESENT_RW | US_FLAG;
        break;

      case 2:
        page_directory[virtual_addr >> PD_SHIFT] = ((uint32_t)vid_page_table & BIT_MASK_UPPER_20) | SET_PRESENT_RW | US_FLAG;
        vid_page_table[(virtual_addr >> PT_SHIFT) & PT_MASK ] =  (VID_B2) | SET_PRESENT_RW | US_FLAG;
        break;

      default:
        break;
    }
  }

  asm volatile (
      // flush TLB
      "movl %%cr3, %%eax;"
      "movl %%eax, %%cr3;"
      // set the paging and protection bits of CR0
      "movl %%cr0, %%eax;"
      "orl $0x80000000, %%eax;"
      "movl %%eax, %%cr0;"
      :
      :
      :"%eax"
    );
}

/* map_video_page(uint32_t addr)
 *
 * Description: remap virtual address to physical address for video memory
 * Inputs: 32-bit address
 * Outputs: None
 * Side Effects: None
 */
void map_video_page(uint32_t addr){
  // set page base address, Present in PTE
  page_table[(addr >> PT_SHIFT)] = addr + SET_PRESENT_RW;

  // flush TLB
  asm volatile (
      // load CR3 with address of the page directory
      "movl %%cr3, %%eax;"
      "movl %%eax, %%cr3;"
      :
      :
      :"%eax"
    );
}


/* map_4KB_page(uint32_t physical)
 *
 * Description: map page table entry to a given physical address
 * Inputs: 32-bit physical address
 * Outputs: None
 * Side Effects: None
 */
void map_4KB_page(uint32_t physical) {
    page_table[VID_MEM_INDEX] = physical + SET_PRESENT_RW;

    // flush TLB
    asm volatile (
        // load CR3 with address of the page directory
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        :"%eax"
      );
}
