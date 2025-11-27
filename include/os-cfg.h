/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#ifndef OSCFG_H
#define OSCFG_H

/* ============================================================
 * Configuration Control for Operating System Simulation
 * ============================================================
 * This file controls feature-oriented program modules through
 * constant definitions. Each feature can be enabled/disabled
 * by uncommenting/commenting the associated #define line.
 * ============================================================
 */

/* ============================================================
 * Scheduler Configuration
 * ============================================================
 */
#define MLQ_SCHED 1        /* Multi-Level Queue Scheduler */
#define MAX_PRIO 140       /* Maximum priority level */

/* ============================================================
 * Memory Management Configuration
 * ============================================================
 */
#define MM_PAGING          /* Enable paging memory management */
                          /* When enabled, adds memory management fields
                           * to PCB/kernel structures */

//#define MM_FIXED_MEMSZ    /* Enable backward compatibility mode */
                          /* When enabled, uses fixed memory sizes:
                           *   MEMRAM = 0x10000000 (256MB)
                           *   MEMSWP[0] = 0x1000000 (16MB)
                           *   MEMSWP[1-3] = 0
                           * When disabled, reads memory sizes from input file:
                           *   Format: [RAM_SZ] [SWP_SZ0] [SWP_SZ1] [SWP_SZ2] [SWP_SZ3]
                           *   Must have RAM > 0 and at least 1 SWP > 0 */

/* 
 * @bksysnet:
 *    The address mode must be explicitly defined in MM64 or no-MM64
 *    by commenting one of these following lines and uncommenting the other
 *      
 */
//#define MM64 1           /* Enable 64-bit memory management structure */
                          /* When enabled, uses 5-level page table:
                           *   pgd, p4d, pud, pmd, pt (all uint64_t)
                           * When disabled, uses 2-level page table:
                           *   pgd (uint32_t) */
#undef MM64

/* ============================================================
 * Debug Configuration
 * ============================================================
 */
//#define VMDBG 1          /* Virtual Memory Debug */
//#define MMDBG 1          /* Memory Management Debug */
#define IODUMP 1           /* I/O Dump */
#define PAGETBL_DUMP 1     /* Page Table Dump */

#endif
