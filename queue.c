#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

static pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;

int empty(struct queue_t *q)
{
        if (q == NULL)
                return 1;
        return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
        /* TODO: put a new process to queue [q] */
        if (q == NULL || proc == NULL)
                return;
        if (q->size == MAX_QUEUE_SIZE)
                q->proc[q->size++] = proc;
        else
                printf("Queue is full.\n");
}

struct pcb_t *dequeue(struct queue_t *q)
{
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        pthread_mutex_lock(&qlock);
        if (empty(q))
        {
                printf("Queue is empty.\n");
                pthread_mutex_unlock(&qlock);
                return NULL;
        }
        else
        {
                struct pcb_t *next = q->proc[0];
                for (int i = 0; i < q->size - 1; i++)
                {
                        q->proc[i] = q->proc[i + 1];
                }

                q->size--;

                pthread_mutex_unlock(&qlock);

                return next;
        }
}
