/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "os-mm.h"
#include "syscall.h"
#include "libmem.h"
#include "queue.h"
#include "sched.h"
#include <stdlib.h>
#include <string.h>

#ifdef MM64
#include "mm64.h"
#else
#include "mm.h"
#endif

//typedef char BYTE;

// Helper function to find a process by pid - used in Task 2.2.3 
static struct pcb_t *find_process_by_pid(struct krnl_t *krnl, uint32_t pid)
{
   struct pcb_t *proc = NULL;
   struct queue_t *running_list = krnl->running_list;
   
   // Search in running_list 
   if (running_list != NULL) {
       for (int i = 0; i < running_list->size; i++) {
           if (running_list->proc[i] != NULL && running_list->proc[i]->pid == pid) {
               return running_list->proc[i];
           }
       }
   }
   
   // Search in ready_queue
   if (krnl->ready_queue != NULL) {
       struct queue_t *ready_queue = krnl->ready_queue;
       for (int i = 0; i < ready_queue->size; i++) {
           if (ready_queue->proc[i] != NULL && ready_queue->proc[i]->pid == pid) {
               return ready_queue->proc[i];
           }
       }
   }

   // Search in MLQ ready queues if available
#ifdef MLQ_SCHED
   if (krnl->mlq_ready_queue != NULL) {
       for (int prio = 0; prio < MAX_PRIO; prio++) {
           struct queue_t *mlq_queue = &(krnl->mlq_ready_queue[prio]);
           if (mlq_queue != NULL) {
               for (int i = 0; i < mlq_queue->size; i++) {
                   if (mlq_queue->proc[i] != NULL && mlq_queue->proc[i]->pid == pid) {
                       return mlq_queue->proc[i];
                   }
               }
           }
       }
   }
#endif
   
   return NULL;
}

// Helper function to find a process by name - used in Task 2.2.4
static struct pcb_t *find_process_by_name(struct krnl_t *krnl, const char *proc_name)
{
   struct pcb_t *proc = NULL;
   struct queue_t *running_list = krnl->running_list;
   
   // Search in running_list
   if (running_list != NULL) {
       for (int i = 0; i < running_list->size; i++) {
           if (running_list->proc[i] != NULL) {
               // Extract process name from path 
               const char *path = running_list->proc[i]->path;
               const char *name = strrchr(path, '/');
               if (name == NULL) name = path;
               else name++; /* Skip the '/' */
               
               if (strcmp(name, proc_name) == 0) {
                   return running_list->proc[i];
               }
           }
       }
   }
   
   // Search in ready_queue
   if (krnl->ready_queue != NULL) {
       struct queue_t *ready_queue = krnl->ready_queue;
       for (int i = 0; i < ready_queue->size; i++) {
           if (ready_queue->proc[i] != NULL) {
               const char *path = ready_queue->proc[i]->path;
               const char *name = strrchr(path, '/');
               if (name == NULL) name = path;
               else name++;
               
               if (strcmp(name, proc_name) == 0) {
                   return ready_queue->proc[i];
               }
           }
       }
   }
   
   // Search in MLQ ready queues if available
#ifdef MLQ_SCHED
   if (krnl->mlq_ready_queue != NULL) {
       for (int prio = 0; prio < MAX_PRIO; prio++) {
           struct queue_t *mlq_queue = &(krnl->mlq_ready_queue[prio]);
           if (mlq_queue != NULL) {
               for (int i = 0; i < mlq_queue->size; i++) {
                   if (mlq_queue->proc[i] != NULL) {
                       const char *path = mlq_queue->proc[i]->path;
                       const char *name = strrchr(path, '/');
                       if (name == NULL) name = path;
                       else name++;
                       
                       if (strcmp(name, proc_name) == 0) {
                           return mlq_queue->proc[i];
                       }
                   }
               }
           }
       }
   }
#endif
   
   return NULL;
}

int __sys_memmap(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
   int memop = regs->a1;
   BYTE value;
   
   /*
    * @bksysnet: Please note in the dual spacing design
    *            syscall implementations are in kernel space.
    */

   // Task 2.2.3: Find the process with matching pid using helper function
   struct pcb_t *caller = find_process_by_pid(krnl, pid);
   
   /* If process not found, return error */
   if (caller == NULL) {
       printf("Error: Process with PID %u not found\n", pid);
       return -1;
   }
	
   switch (memop) {
   case SYSMEM_MAP_OP:
            /* Reserved process case*/
			vmap_pgd_memset(caller, regs->a2, regs->a3);
            break;
   case SYSMEM_INC_OP:
            inc_vma_limit(caller, regs->a2, regs->a3);
            break;
   case SYSMEM_SWP_OP:
            __mm_swap_page(caller, regs->a2, regs->a3);
            break;
   case SYSMEM_IO_READ:
            MEMPHY_read(caller->krnl->mram, regs->a2, &value);
            regs->a3 = value;
            break;
   case SYSMEM_IO_WRITE:
            MEMPHY_write(caller->krnl->mram, regs->a2, regs->a3);
            break;
   default:
            printf("Memop code: %d\n", memop);
            break;
   }
   
   return 0;
}


