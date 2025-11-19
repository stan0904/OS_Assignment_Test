/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

static struct queue_t running_list;
#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];

/* Biến tĩnh để theo dõi trạng thái MLQ */
static int current_prio = 0; 
static int current_slot = 0;
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
    unsigned long prio;
    for (prio = 0; prio < MAX_PRIO; prio++)
        if(!empty(&mlq_ready_queue[prio])) 
            return -1;
#endif
    return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
    int i ;

    for (i = 0; i < MAX_PRIO; i ++) {
        mlq_ready_queue[i].size = 0;
        slot[i] = MAX_PRIO - i; 
    }
#endif
    ready_queue.size = 0;
    run_queue.size = 0;
    running_list.size = 0;
    pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/* * Stateful design for routine calling
 * based on the priority and our MLQ policy
 * We implement stateful here using transition technique
 * State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
    struct pcb_t * proc = NULL;

    pthread_mutex_lock(&queue_lock);

    // 1. Kiểm tra xem tất cả các hàng đợi có rỗng không
    int all_empty = 1;
    for (int i = 0; i < MAX_PRIO; i++) {
        if (!empty(&mlq_ready_queue[i])) {
            all_empty = 0; // Tìm thấy một hàng đợi không rỗng
            break;
        }
    }

    if (all_empty) {
        pthread_mutex_unlock(&queue_lock); // SỬA LỖI DEADLOCK
        return NULL; // Không có tiến trình nào để chạy
    }

    // 2. Vòng lặp tìm tiến trình theo chính sách MLQ
    while (proc == NULL) {
        if (!empty(&mlq_ready_queue[current_prio])) {
            // Hàng đợi hiện tại có tiến trình

            if (current_slot < slot[current_prio]) {
                // Hàng đợi này vẫn còn slot
                proc = dequeue(&mlq_ready_queue[current_prio]);
                current_slot++; // Tăng số slot đã dùng
            } else {
                // Hàng đợi này đã hết slot
                current_slot = 0; // Reset số slot đã dùng
                current_prio = (current_prio + 1) % MAX_PRIO; // Chuyển sang mức ưu tiên tiếp theo
            }
        } else {
            // Hàng đợi hiện tại rỗng, chuyển sang mức ưu tiên tiếp theo
            current_slot = 0; // Reset số slot đã dùng
            // slot[current_prio] = MAX_PRIO - current_prio; // (Không cần reset slot ở đây)
            current_prio = (current_prio + 1) % MAX_PRIO;
        }
    }

    /* Thêm tiến trình vào running_list (để quản lý các tiến trình đang chạy) */
    if (proc != NULL) {
        enqueue(&running_list, proc);
    }

    pthread_mutex_unlock(&queue_lock); // SỬA LỖI DEADLOCK
    return proc;
}

void put_mlq_proc(struct pcb_t * proc) {
    pthread_mutex_lock(&queue_lock);

    /* Xóa tiến trình khỏi running_list */
    purgequeue(&running_list, proc);

    /* Thêm tiến trình trở lại hàng đợi ưu tiên của nó */
    enqueue(&mlq_ready_queue[proc->prio], proc);

    pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
    /*
     * Hàm này dành cho tiến trình MỚI (từ loader), 
     * nó chưa có trong running_list.
     */
    proc->krnl->ready_queue = &ready_queue;
    proc->krnl->mlq_ready_queue = mlq_ready_queue;
    proc->krnl->running_list = &running_list;

    pthread_mutex_lock(&queue_lock);

    /* Chỉ cần thêm vào hàng đợi ưu tiên của nó */
    enqueue(&mlq_ready_queue[proc->prio], proc);

    pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
    return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
    return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
    return add_mlq_proc(proc);
}
#else
/* * PHẦN NÀY KHÔNG ĐƯỢC SỬ DỤNG VÌ MLQ_SCHED ĐANG BẬT
 * (Để nguyên code cũ ở đây cũng không sao)
 */
struct pcb_t * get_proc(void) {
    struct pcb_t * proc = NULL;

    pthread_mutex_lock(&queue_lock);
    /*TODO: get a process from [ready_queue].
     * It worth to protect by a mechanism.
     * */

    pthread_mutex_unlock(&queue_lock);

    return proc;
}

void put_proc(struct pcb_t * proc) {
    proc->krnl->ready_queue = &ready_queue;
    proc->krnl->running_list = &running_list;

    /* TODO: put running proc to running_list 
     * It worth to protect by a mechanism.
     * */

    pthread_mutex_lock(&queue_lock);
    enqueue(&run_queue, proc);
    pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
    proc->krnl->ready_queue = &ready_queue;
    proc->krnl->running_list = &running_list;

    /* TODO: put running proc to running_list 
     * It worth to protect by a mechanism.
     * */

    pthread_mutex_lock(&queue_lock);
    enqueue(&ready_queue, proc);
    pthread_mutex_unlock(&queue_lock);	
}
#endif
