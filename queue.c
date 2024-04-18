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
  q->max_size = size; // Set the maximum size of the queue
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
    // If both head and tail indices are -1, the queue is empty
    return (q->head == -1 && q->tail == -1);
    // or
    //   return (q->n_elements == 0);

}

int queue_full(queue *q)
{
  // If tail index is equal to the maximum max_size of the queue, the queue is full
  return (q->tail == q->max_size - 1);
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
  free(q->buffer); // Free the memory allocated for the buffer
  free(q); // Free the memory allocated for the queue structure
  return 0;
}

// Just for testing purporses, delete later
/*int main() {
    // Create a queue
    queue *q = queue_init(5); // Assuming a queue max_size of 5

    // Create some elements to enqueue
    struct element e1 = {1,0, 10};
    struct element e2 = {2, 1, 5};
    struct element e3 = {3, 0, 8};

    // Enqueue elements
    queue_put(q, &e1);
  

    // Print the contents of the queue
    printf("Queue Contents:\n");
    for (int i = q->head; i != q->tail; i = (i + 1) % q->max_size) {
        printf("Product ID: %d, Operation: %d, Units: %d\n", 
                q->buffer[i].product_id, q->buffer[i].op, q->buffer[i].units);
    }

    queue_get(q);

    printf("Queue Contents after dequeuing 1:\n");
    for (int i = q->head; i != q->tail; i = (i + 1) % q->max_size) {
        printf("Product ID: %d, Operation: %d, Units: %d\n", 
                q->buffer[i].product_id, q->buffer[i].op, q->buffer[i].units);
    }



    queue_get(q);

    printf("Queue Contents after dequeuing 2:\n");
    for (int i = q->head; i != q->tail; i = (i + 1) % q->max_size) {
        printf("Product ID: %d, Operation: %d, Units: %d\n", 
                q->buffer[i].product_id, q->buffer[i].op, q->buffer[i].units);
    }

    queue_get(q);
    printf("Queue Contents after dequeuing 3:\n");
    for (int i = q->head; i != q->tail; i = (i + 1) % q->max_size) {
        printf("Product ID: %d, Operation: %d, Units: %d\n", 
                q->buffer[i].product_id, q->buffer[i].op, q->buffer[i].units);
    }

    queue_get(q);




    // Destroy the queue
    queue_destroy(q);

    return 0;
}*/