/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

#if !defined(MM64)

/*
 * init_pte - Initialize PTE entry
 */
int init_pte(addr_t *pte,
             int pre,    // present
             addr_t fpn,    // FPN
             int drt,    // dirty
             int swp,    // swap
             int swptyp, // swap type
             addr_t swpoff) // swap offset
{
  if (pre != 0) {
    if (swp == 0) { // Non swap ~ page online
      if (fpn == 0) return -1; 
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);
      SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
    } else { // page swapped
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);
      SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
      SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);
    }
  }
  return 0;
}

int pte_set_swap(struct pcb_t *caller, addr_t pgn, int swptyp, addr_t swpoff)
{
  addr_t *pte = &caller->mm->pgd[pgn];
  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);
  SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
  SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);
  return 0;
}

int pte_set_fpn(struct pcb_t *caller, addr_t pgn, addr_t fpn)
{
  addr_t *pte = &caller->mm->pgd[pgn];
  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);
  SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
  return 0;
}

uint32_t pte_get_entry(struct pcb_t *caller, addr_t pgn)
{
  return caller->mm->pgd[pgn];
}

int pte_set_entry(struct pcb_t *caller, addr_t pgn, uint32_t pte_val)
{
  caller->mm->pgd[pgn] = pte_val;
  return 0;
}

/*
 * vmap_page_range - map a range of page at aligned address
 */
addr_t vmap_page_range(struct pcb_t *caller,           
                    addr_t addr,                       
                    int pgnum,                      
                    struct framephy_struct *frames, 
                    struct vm_rg_struct *ret_rg)    
{                                                   
  addr_t pgn;
  addr_t fpn;
  struct framephy_struct *fpit = frames;
  int pgit = 0;

  ret_rg->rg_start = addr;
  ret_rg->rg_end = addr + pgnum * PAGING_PAGESZ;

  for (pgit = 0; pgit < pgnum; pgit++)
  {
      if (fpit == NULL) {
          printf("Error: vmap_page_range - Missing frames!\n");
          break; 
      }
      pgn = PAGING_PGN(addr) + pgit;
      fpn = fpit->fpn;

      pte_set_fpn(caller, pgn, fpn);
      enlist_pgn_node(&caller->mm->fifo_pgn, pgn);

      fpit = fpit->fp_next;
  }
  return 0;
}

/*
 * alloc_pages_range - allocate req_pgnum of frame in ram
 */
addr_t alloc_pages_range(struct pcb_t *caller, int req_pgnum, struct framephy_struct **frm_lst)
{
  int pgit = 0;
  addr_t fpn;
  struct framephy_struct *newfp_str = NULL;
  struct framephy_struct *head = NULL;

  for (pgit = 0; pgit < req_pgnum; pgit++)
  {
    if (MEMPHY_get_freefp(caller->mram, &fpn) == 0)
    {
       newfp_str = malloc(sizeof(struct framephy_struct));
       newfp_str->fpn = fpn;
       newfp_str->fp_next = head;
       head = newfp_str;
    }
    else
    { 
       /* Trường hợp RAM đầy -> Phải SWAP */
       printf("RAM đầy, thực hiện thay thế trang...\n");
       
       /* KHAI BÁO BIẾN DUY NHẤT Ở ĐÂY */
       addr_t vicpgn, swpfpn; 
       
       // Tìm trang nạn nhân
       if (find_victim_page(caller->mm, &vicpgn) < 0) {
           printf("Error: Cannot find victim page! (Process %d)\n", caller->pid);
           return -1;
       }

       addr_t vicpte = pte_get_entry(caller, vicpgn);
       addr_t vicfpn = PAGING_FPN(vicpte);

       // Tìm chỗ trống trong SWAP
       if (MEMPHY_get_freefp(caller->active_mswp, &swpfpn) < 0) {
           printf("Error: SWAP full!\n");
           return -1;
       }

       printf("SWAP OUT: PGN %d (FPN %d) -> SWAP_FPN %d\n", vicpgn, vicfpn, swpfpn);
       
       // Thực hiện SWAP
       __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
       pte_set_swap(caller, vicpgn, 0, swpfpn);
    
    printf("\n>>> [SWAP HAPPENDED] Victim PGN: %d pushed to SWAP frame %d <<<\n", vicpgn, swpfpn);
    
       // Lấy frame vừa giải phóng để dùng
       fpn = vicfpn;
       newfp_str = malloc(sizeof(struct framephy_struct));
       newfp_str->fpn = fpn;
       newfp_str->fp_next = head;
       head = newfp_str;
    }
  }

  *frm_lst = head;
  return 0;
}

addr_t vm_map_ram(struct pcb_t *caller, addr_t astart, addr_t aend, addr_t mapstart, int incpgnum, struct vm_rg_struct *ret_rg)
{
  struct framephy_struct *frm_lst = NULL;
  addr_t ret_alloc = 0;

  ret_alloc = alloc_pages_range(caller, incpgnum, &frm_lst);

  if (ret_alloc < 0) {
    printf("Error: alloc_pages_range failed.\n");
    return -1;
  }
  
  vmap_page_range(caller, mapstart, incpgnum, frm_lst, ret_rg);
  return 0;
}

int __swap_cp_page(struct memphy_struct *mpsrc, addr_t srcfpn,
                   struct memphy_struct *mpdst, addr_t dstfpn)
{
  int cellidx;
  addr_t addrsrc, addrdst;
  BYTE data;

  for (cellidx = 0; cellidx < PAGING_PAGESZ; cellidx++)
  {
    addrsrc = srcfpn * PAGING_PAGESZ + cellidx;
    addrdst = dstfpn * PAGING_PAGESZ + cellidx;

    if (MEMPHY_read(mpsrc, addrsrc, &data) != 0) return -1;
    if (MEMPHY_write(mpdst, addrdst, data) != 0) return -1;
  }
  return 0;
}

int init_mm(struct mm_struct *mm, struct pcb_t *caller)
{
  struct vm_area_struct *vma0 = malloc(sizeof(struct vm_area_struct));

  mm->pgd = (uint32_t *)malloc(PAGING_MAX_PGN * sizeof(uint32_t));
  if (mm->pgd == NULL) return -1;
  memset(mm->pgd, 0, PAGING_MAX_PGN * sizeof(uint32_t));

  mm->fifo_pgn = NULL;
  memset(mm->symrgtbl, 0, sizeof(struct vm_rg_struct) * PAGING_MAX_SYMTBL_SZ);

  vma0->vm_id = 0;
  vma0->vm_start = 0;
  vma0->vm_end = vma0->vm_start;
  vma0->sbrk = vma0->vm_start;
  vma0->vm_freerg_list = NULL;
  vma0->vm_next = NULL;
  vma0->vm_mm = mm; 

  mm->mmap = vma0;
  return 0;
}

/* Dummy functions for 32-bit mode */
int get_pd_from_address(addr_t addr, addr_t* pgd, addr_t* p4d, addr_t* pud, addr_t* pmd, addr_t* pt) { return 0; }
int get_pd_from_pagenum(addr_t pgn, addr_t* pgd, addr_t* p4d, addr_t* pud, addr_t* pmd, addr_t* pt) { return 0; }
int vmap_pgd_memset(struct pcb_t *caller, addr_t addr, int pgnum) { return 0; }

/* Helper functions */
struct vm_rg_struct *init_vm_rg(addr_t rg_start, addr_t rg_end) {
  struct vm_rg_struct *rgnode = malloc(sizeof(struct vm_rg_struct));
  rgnode->rg_start = rg_start; rgnode->rg_end = rg_end; rgnode->rg_next = NULL;
  return rgnode;
}
int enlist_vm_rg_node(struct vm_rg_struct **rglist, struct vm_rg_struct *rgnode) {
  rgnode->rg_next = *rglist; *rglist = rgnode; return 0;
}
int enlist_pgn_node(struct pgn_t **plist, addr_t pgn) {
  struct pgn_t *pnode = malloc(sizeof(struct pgn_t));
  pnode->pgn = pgn; pnode->pg_next = *plist; *plist = pnode;
  return 0;
}

/* SỬA LỖI: Các hàm print_list phải dùng CON TRỎ (*) để khớp với .h */
int print_list_fp(struct framephy_struct *ifp) { return 0; }
int print_list_rg(struct vm_rg_struct *irg) { return 0; }
int print_list_vma(struct vm_area_struct *ivma) { return 0; }
int print_list_pgn(struct pgn_t *ip) { return 0; }
int print_pgtbl(struct pcb_t *caller, uint32_t start, uint32_t end) { return 0; }

#endif //ndef MM64
