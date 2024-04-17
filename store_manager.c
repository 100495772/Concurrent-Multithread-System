//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_FILENAME_LENGTH 100 // Maximum length of the file name
#define MAX_OPERATION_LENGTH 9 // Operation type: PURCHASE or SALE


struct Transaction {
    int product_id;
    const char operation[MAX_OPERATION_LENGTH]; // Operation type: PURCHASE or SALE
    int units;
};


// Structure used to pass parameters to the threads
struct ThreadArgs {
    int id; 
    int start_index; // Start index of operations for this thread
    int end_index;   // End index of operations for this thread
    struct Transaction *operations;
    int buffer_size;
        queue* q; // Pointer to the queue

    
};

// Producer
void* producer(void* args) {
  struct ThreadArgs* pr_args = (struct ThreadArgs*)args;
    // Access thread arguments
    int id = pr_args->id;
    struct Transaction *operations = pr_args->operations;
    int buffer_size = pr_args->buffer_size;
    int start_index = pr_args->start_index;
    int end_index = pr_args->end_index;

  // Process operations assigned to this thread
  for (int i = start_index; i < end_index; i++) {
            
    // Create an element with the operation data to insert in the queue.
    // Allocate memory for the element
    struct element *elem = malloc(sizeof(struct element));
    if (elem == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        pthread_exit(NULL);
    }
    
    // Set element data
    elem->product_id = operations[i].product_id;
    elem->units = operations[i].units;

    // Convert operation string to integer
    if (strcmp(operations[i].operation, "PURCHASE") == 0) {
        elem->op = 0;
    } else if (strcmp(operations[i].operation, "SALE") == 0) {
        elem->op = 1;
    } else {
        printf("Invalid operation\n");
        free(elem);
        exit(1);
    }

    // Insert element in queue
    queue_put(pr_args->q, elem);
    
    printf("Producer thread %d processing operation %d\n",pr_args->id, i);

    // Free allocated memory for the element
    free(elem);

  }

  // Free memory allocated for thread arguments
  free(pr_args);

  // Exit the thread
  pthread_exit(NULL);
}


int main (int argc, const char * argv[])
{
  // check number of arguments
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <file_name> <num_producers> <num_consumers> <buffer_size>\n", argv[0]);
    return 1;
  }

  int profits = 0;
  int product_stock [5] = {0};

  // get arguments
  char filename[MAX_FILENAME_LENGTH];
  FILE *file;
  strcpy(filename, argv[1]);

  int num_producers = atoi(argv[2]);
  int num_consumers = atoi(argv[3]);
  int buffer_size = atoi(argv[4]);

  // Initialize the queue (circular buffer)
  queue * q = queue_init(buffer_size);
  if (q == NULL) {
    // Error
    fprintf(stderr, "Error initilizing queue");
    return 1;
  }

  // Open the file
  file = fopen(filename, "r");
  if (file == NULL) {
      printf("Error opening file %s\n", filename);
      return 1;
  }

  // Read the number of operations
  int num_operations;
  if (fscanf(file, "%d", &num_operations) != 1) {
      printf("Error reading the number of operations from file %s\n", filename);
      fclose(file);
      return 1;
  }
    
  printf("number of operations is: %d\n", num_operations);

  // Allocate memory for operations
  struct Transaction *operations = malloc(num_operations * sizeof(struct Transaction));
  if (operations == NULL) {
      printf("Memory allocation failed\n");
      fclose(file);
      return 1;
  }

  // Read operations from the file
  for (int i = 0; i < num_operations; i++) {
      if (fscanf(file, "%d %s %d", &operations[i].product_id, (char *)&operations[i].operation, &operations[i].units) != 3) {
        printf("Error reading operation %d from file %s\n", i + 1, filename);
        fclose(file);
        free(operations);
        return 1;
      }
    printf("Operation %d: Product ID: %d, Operation Type: %s, Units: %d\n", i + 1, operations[i].product_id, operations[i].operation, operations[i].units);
  }

  // Close the file
    fclose(file);

  // Now operations array contains the operations read from the file

  // Distribute operations equally among producer threads
  int operations_per_thread = num_operations / num_producers;
  int remaining_operations = num_operations % num_producers;
  int start_index = 0;

  pthread_t producers[num_producers];

  for (int i = 0; i < num_producers; i++) {
    struct ThreadArgs* thread_args = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs));
    if (thread_args == NULL) {
      fprintf(stderr, "Error: Memory allocation failed\n");
      free(operations);
      return 1;
    }

    thread_args->id = i;
    thread_args->operations = operations;
    thread_args->buffer_size = buffer_size;
    thread_args->start_index = start_index;
    thread_args->q = q; // Pass the queue pointer


    /* producer threads are assigned operations in such a way that the
    first remaining_operations threads receive one additional operation compared to the rest, 
    to ensure fair distribution*/
    if (remaining_operations > 0) {
      thread_args->end_index = start_index + operations_per_thread + 1;
      remaining_operations--;   // Decrement the number of remaining operations
    // We are assigning the arguments of the last thread 
    } else {
        thread_args->end_index = start_index + operations_per_thread;
    }

    // Update the start index for the next thread
    start_index = thread_args->end_index;

    // Launch producer thread
    if (pthread_create(&producers[i], NULL, producer, (void*)thread_args) != 0) {
      fprintf(stderr, "Error: Unable to create producer thread\n");
      free(operations);
      free(thread_args);
      return 1;
    }
  }

  // Wait for producer threads to finish
  for (int i = 0; i < num_producers; i++) {
    if (pthread_join(producers[i], NULL) != 0) {
      fprintf(stderr, "Error: Unable to join producer thread\n");
      free(operations);
      return 1;
    }
  }









    







  // Output
  printf("Total: %d euros\n", profits);
  printf("Stock:\n");
  printf("  Product 1: %d\n", product_stock[0]);
  printf("  Product 2: %d\n", product_stock[1]);
  printf("  Product 3: %d\n", product_stock[2]);
  printf("  Product 4: %d\n", product_stock[3]);
  printf("  Product 5: %d\n", product_stock[4]);

  // Free allocated memory
  free(operations);

  // Destroy the queue
  queue_destroy(q);

  return 0;
}
