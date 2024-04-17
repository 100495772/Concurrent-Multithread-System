//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"

//To create a queue
queue* queue_init(int size)
{
  queue *q = (queue *)malloc(sizeof(queue)); // Allocate memory for the queue structure
  if (q == NULL) {
      return NULL;
  }
  q->buffer = (struct element *)malloc(size * sizeof(struct element)); // Allocate memory for the buffer
  if (q->buffer == NULL) {
      free(q);
      return NULL;
  }
  q->head = -1; // Initialize head index
  q->tail = -1; // Initialize tail index
  q->size = size; // Set the maximum size of the queue
  return q;
}

// To Enqueue an element
int queue_put(queue *q, struct element* x)
{
  if (q == NULL || x == NULL) { 
        perror("Invalid queue or element"); 
        return -1; 
  }

  if (queue_full(q)) {
      perror("Queue is full");
      return -1; 
  }

  // Add element to the queue
  q->buffer[q->tail] = *x;
  q->tail = (q->tail + 1) % q->size; // Move tail pointer circularly

  return 0;
}

// To Dequeue an element.
struct element* queue_get(queue *q)
{
  // Check if the queue is empty
  if (queue_empty(q)) {
      return NULL; // Return NULL if the queue is empty
  }

  // Get the element at the head of the queue
  struct element* element = &(q->buffer[q->head]);

  // Move the head index forward
  q->head++;

  // If the queue becomes empty after dequeueing, reset head and tail indices
  if (q->head > q->tail) {
      q->head = -1;
      q->tail = -1;
  }

  return element;
}

//To check queue state
int queue_empty(queue *q)
{
  // If head index is -1, the queue is empty
  return (q->head == -1);
}

int queue_full(queue *q)
{
  // If tail index is equal to the maximum size of the queue, the queue is full
  return (q->tail == q->size - 1);
}

//To destroy the queue and free the resources
int queue_destroy(queue *q)
{
  if (q == NULL) {
      perror("Queue is a null pointer");
      return -1;
  }
  free(q->buffer); // Free the memory allocated for the buffer
  free(q); // Free the memory allocated for the queue structure
  return 0;
}
