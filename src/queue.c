#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q)
{
    if (q == NULL)
        return 1;
    return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
    /* Thêm một tiến trình vào cuối hàng đợi */
    if (q == NULL || proc == NULL) return;

    if (q->size < MAX_QUEUE_SIZE) {
        q->proc[q->size] = proc;
        q->size++;
    } else {
        printf("Error: Queue is full!\n");
    }
}

struct pcb_t *dequeue(struct queue_t *q)
{
    /* Lấy tiến trình từ đầu hàng đợi (FIFO) */
    if (q == NULL || empty(q)) {
        return NULL;
    }

    struct pcb_t *proc = q->proc[0];

    // Dịch chuyển các phần tử còn lại lên trước
    for (int i = 0; i < q->size - 1; i++) {
        q->proc[i] = q->proc[i + 1];
    }

    q->proc[q->size - 1] = NULL; // Xóa con trỏ ở vị trí cuối
    q->size--;

    return proc;
}

struct pcb_t *purgequeue(struct queue_t *q, struct pcb_t *proc)
{
    /* Xóa một tiến trình cụ thể khỏi hàng đợi */
    if (q == NULL || proc == NULL || empty(q)) {
        return NULL;
    }

    int found_index = -1;
    for (int i = 0; i < q->size; i++) {
        if (q->proc[i] == proc) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Dịch chuyển các phần tử từ vị trí tìm thấy
        for (int i = found_index; i < q->size - 1; i++) {
            q->proc[i] = q->proc[i + 1];
        }
        q->proc[q->size - 1] = NULL;
        q->size--;
        return proc;
    }

    return NULL; // Không tìm thấy
}
