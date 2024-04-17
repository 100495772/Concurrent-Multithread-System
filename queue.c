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
  q->front = -1; // Initialize front index
  q->rear = -1; // Initialize rear index
  q->size = size; // Set the maximum size of the queue
  return q;
}

// To Enqueue an element
int queue_put(queue *q, struct element* x)
{
  if (q == NULL || x == NULL) {
        return -1; // Invalid queue or element
  }

  if (queue_full(q)) {
      return -2; // Queue is full
  }

  // Add element to the queue
  q->buffer[q->rear] = *x;
  q->rear = (q->rear + 1) % q->size; // Move rear pointer circularly

  return 0;
}

// To Dequeue an element.
struct element* queue_get(queue *q)
{
  // Check if the queue is empty
  if (queue_empty(q)) {
      return NULL; // Return NULL if the queue is empty
  }

  // Get the element at the front of the queue
  struct element* element = &(q->buffer[q->front]);

  // Move the front index forward
  q->front++;

  // If the queue becomes empty after dequeueing, reset front and rear indices
  if (q->front > q->rear) {
      q->front = -1;
      q->rear = -1;
  }

  return element;
}

//To check queue state
int queue_empty(queue *q)
{
  // If front index is -1, the queue is empty
  return (q->front == -1);
}

int queue_full(queue *q)
{
  // If rear index is equal to the maximum size of the queue, the queue is full
  return (q->rear == q->size - 1);
}

//To destroy the queue and free the resources
int queue_destroy(queue *q)
{
  if (q == NULL) {
      // Error handling for NULL pointer
      return -1;
  }
  free(q->buffer); // Free the memory allocated for the buffer
  free(q); // Free the memory allocated for the queue structure
  return 0;
}
