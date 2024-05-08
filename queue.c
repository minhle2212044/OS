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
	{
		fprintf(stderr, "Queue is full.\n");
		exit(1);
	}
	else
	{
		q->proc[q->size++] = proc;
	}
}

struct pcb_t *dequeue(struct queue_t *q)
{
	/* TODO: return a pcb whose prioprity is the highest
	 * in the queue [q] and remember to remove it from q
	 * */
	pthread_mutex_lock(&qlock);
	if (empty(q))
	{
		fprintf(stderr, "Queue is empty.\n");
		pthread_mutex_unlock(&qlock);
		return NULL;
	}
	else
	{
		int i, max = 0;
		uint32_t max_prio = q->proc[max]->priority;
		if (q->proc[max]->prio)
		{
			max_prio = q->proc[max]->prio;
		}
		for (i = 1; i < q->size; i++)
		{
			if (q->proc[i]->prio)
			{
				if (q->proc[i]->prio > max_prio)
				{
					max = i;
					max_prio = q->proc[i]->prio;
				}
			}
			else
			{
				if (q->proc[i]->priority > max_prio)
				{
					max = i;
					max_prio = q->proc[i]->priority;
				}
			}
		}
		struct pcb_t *out_proc = q->proc[max];
		for (i = max; i < q->size - 1; i++)
		{
			q->proc[i] = q->proc[i + 1];
		}
		q->size--;
		pthread_mutex_unlock(&qlock);
		return out_proc;
	}
}
