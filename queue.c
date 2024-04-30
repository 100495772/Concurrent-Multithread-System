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
  q->head = 0; // Initialize head index
  q->tail = 0; // Initialize tail index
  q->max_size = size; // Set the maximum size of the queue
  q->cur_size = 0; // Initialize the current size of the queue
  //q->n_elements = 0; // Initialize number of elements to 0
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
  q->tail = (q->tail + 1) % q->max_size; // Move tail pointer circularly
  q->cur_size++; // Increment the current size of the queue

  return 0;
}

// To Dequeue an element.
struct element* queue_get(queue *q)
{
  // Check if the queue is empty
  if (queue_empty(q)) {
      printf("Cannot dequeue, queue is empty\n");
      return NULL; // Return NULL if the queue is empty
  }

  // Allocate memory for the element
  struct element* element = (struct element*)malloc(sizeof(struct element));
  if (element == NULL) {
      // Handle memory allocation failure
      perror("Memory allocation for element failed");
      return NULL;
  }

  // Copy the element at the head of the queue to the newly allocated memory
  *element = q->buffer[q->head];

  // Move the head index forward
  q->head = (q->head + 1) % q->max_size;
  q->cur_size--; // Decrement the current size of the queue

  // If the queue becomes empty after dequeueing, reset head and tail indices
  if (q->cur_size == 0) {
      q->head = 0;
      q->tail = 0;
  }

  return element;
}

//To check queue state
int queue_empty(queue *q)
{
    // If both head and tail indices are -1, the queue is empty
    return (q->cur_size == 0);
    // or
    //   return (q->n_elements == 0);

}

int queue_full(queue *q)
{
  // If tail index is equal to the maximum max_size of the queue, the queue is full
  return (q->cur_size == q->max_size);
  // or
  // return (q->n_elements == q->max_size);

}

//To destroy the queue and free the resources
int queue_destroy(queue *q)
{
  if (q == NULL) {
      perror("Queue is a null pointer");
      return -1;
  }
  // free(q->buffer); // Free the memory allocated for the buffer
  free(q); // Free the memory allocated for the queue structure
  return 0;
}
