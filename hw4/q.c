//https://www.geeksforgeeks.org/queue-set-2-linked-list-implementation/

// A C program to demonstrate linked list based implementation of queue 
#include <stdlib.h> 
#include <stdio.h> 

struct Human
{
  int timeWaiting;
};
  
// A linked list (LL) node to store a queue entry 
struct QNode 
{ 
    struct Human *human; 
    struct QNode *next; 
}; 
  
// The queue, front stores the front node of LL and rear stores ths 
// last node of LL 
struct Queue 
{ 
    struct QNode *front, *rear; 
}; 
  
// A utility function to create a new linked list node. 
struct QNode* newNode(struct Human *human) 
{ 
    struct QNode *temp = (struct QNode*)malloc(sizeof(struct QNode)); 
    temp->human = human; 
    temp->next = NULL; 
    return temp;  
} 
  
// A utility function to create an empty queue 
struct Queue *createQueue() 
{ 
    struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue)); 
    q->front = q->rear = NULL; 
    return q; 
} 
  
// The function to add a key k to q 
void push(struct Queue *q, struct Human *h) 
{ 
    // Create a new LL node 
    struct QNode *temp = newNode(h); 
  
    // If queue is empty, then new node is front and rear both 
    if (q->rear == NULL) 
    { 
       q->front = q->rear = temp; 
       return; 
    } 
  
    // Add the new node at the end of queue and change rear 
    q->rear->next = temp; 
    q->rear = temp; 
} 
  
// Function to remove a key from given queue q 
struct Human *pop(struct Queue *q) 
{ 
    // If queue is empty, return NULL. 
    if (q->front == NULL) 
       return NULL; 
  
    // Store previous front and move front one node ahead 
    struct Human *temp = q->front->human; 
    q->front = q->front->next; 
  
    // If front becomes NULL, then change rear also as NULL 
    if (q->front == NULL) 
       q->rear = NULL; 
    return temp; 
} 

void waitingTimeUpdater(struct QNode *node)
{
    if (node != NULL)
	{
		node->human->timeWaiting = node->human->timeWaiting + 1;
    	waitingTimeUpdater(node->next);
	}
}

void updateWaitingTimes(struct Queue *q)
{
	waitingTimeUpdater(q->front);
}

int sumWaitTime(int x, struct QNode *node)
{
    if (x > 0)
	{
    	if (node != NULL)
		{
    		return sumWaitTime(x-1,node->next) + node->human->timeWaiting;
		}
	}
	else
    {
		return 0;
	}
}

int sumXWaitingTimes(int x, struct Queue *q)
{
  sumWaitTime(x,q->front);
}

// Driver Program to test anove functions 
int main() 
{
    struct Queue *q = createQueue(); 
	updateWaitingTimes(q);
	struct Human *t1 = (struct Human*)malloc(sizeof(struct Human));
    push(q, t1);

	updateWaitingTimes(q);
	struct Human *t2 = (struct Human*)malloc(sizeof(struct Human));
    push(q, t2);

	updateWaitingTimes(q);
	struct Human *t3 = (struct Human*)malloc(sizeof(struct Human));
    push(q, t3);

	printf("%d\n",sumXWaitingTimes(3,q));
    struct Human *n = pop(q); 
    if (n != NULL) 
      printf("Dequeued item is %d\n", n->timeWaiting);

    

    return 0; 
} 

