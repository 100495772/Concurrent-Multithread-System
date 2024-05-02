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

pthread_mutex_t mutex;   /* mutex to access shared buffer */
pthread_cond_t non_full; /* can we add more elements? */
pthread_cond_t non_empty; /* can we remove elements? */
pthread_mutex_t mremaining_producers;
int remaining_producers; 


int purchase_cost[] = {2, 5, 15, 25, 100};
int selling_price[] = {3, 10, 20, 40, 125};

struct Transaction {
    int product_id;
    const char operation[MAX_OPERATION_LENGTH]; // Operation type: PURCHASE or SALE
    int units;
};


// Structure used to pass parameters to the producer threads
struct producer_args {
    int id; 
    int start_index; // Start index of operations for this thread
    int end_index;   // End index of operations for this thread
    struct Transaction *operations;
    int buffer_size;
    queue* q; // Pointer to the queue


};

struct consumer_result {
  int profit;
  int partial_stock[5];
};

// Consumer
void* consumer(void * args) {
  
    queue *q = (struct queue*)args;

    struct consumer_result *c_result = (struct consumer_result*)malloc(sizeof(struct consumer_result));
    c_result->profit = 0;
    for (int i = 0; i < 5; i++){
      c_result->partial_stock[i] = 0;
    }

    for (;;) {
      pthread_mutex_lock(&mutex);
      while (queue_empty(q)) {
        if (remaining_producers == 0 && queue_empty(q)) {
          pthread_mutex_unlock(&mutex);
          // Consumer must return the profit and partial stock 
          pthread_exit((void*)c_result);
        }
        pthread_cond_wait(&non_empty, &mutex);
      } 
      struct element *element = queue_get(q);
      pthread_cond_signal(&non_full);
      pthread_mutex_unlock(&mutex);

      // Reply request
      // Extract product ID, operation type, and units from the element
      int product_id = element->product_id;
      int operation = element->op;
      int units = element->units;



      // Calculate profit and update stock based on the operation type
      if (operation == 0) { // Purchase operation
        
          c_result->profit -= units * purchase_cost[product_id - 1];
          c_result->partial_stock[product_id - 1] += units;
          
      } else if (operation == 1) { // Sale operation

          c_result->profit += units * selling_price[product_id - 1];
          c_result->partial_stock[product_id - 1] -= units;
          
      }

      // Free the memory for the dequeued element (the memory allocation is made in the queue_get())
      free(element);
    }
}


// Producer
void* producer(void* args) {

  struct producer_args* pr_args = (struct producer_args*)args;
  // Access thread arguments
  // int id = pr_args->id;
  struct Transaction *operations = pr_args->operations;
  // int buffer_size = pr_args->buffer_size;
  int start_index = pr_args->start_index;
  int end_index = pr_args->end_index;
  queue* q = pr_args->q;

  // Process operations assigned to this thread
  for (int i = start_index; i < end_index; i++) {

    // Create an element with the operation data to insert in the queue.
    int local_operation;
    // Convert operation string to integer
    if (strcmp(operations[i].operation, "PURCHASE") == 0) {
        local_operation = 0;
    } else if (strcmp(operations[i].operation, "SALE") == 0) {
        local_operation = 1;
    } else {
        printf("Invalid operation\n");
        exit(1);
    }

    struct element elem = {operations[i].product_id, local_operation,operations[i].units };

    pthread_mutex_lock(&mutex);
    while( queue_full(q)) {
      pthread_cond_wait(&non_full, &mutex);
    } 

    // Insert element in queue
    queue_put(q, &elem);

    pthread_cond_signal(&non_empty);
    pthread_mutex_unlock(&mutex);


  }

  // fprintf(stderr,"Finishing producer\n");
  pthread_mutex_lock(&mremaining_producers);
  remaining_producers -= 1;
  pthread_mutex_unlock(&mremaining_producers);
  pthread_mutex_lock(&mutex);
  pthread_cond_broadcast(&non_empty);
  pthread_mutex_unlock(&mutex);

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
  // Array of size of 5 elements, all initialized to 0
  int product_stock [5] = {0};

  pthread_mutex_init(&mutex,NULL);
  pthread_cond_init(&non_full,NULL);
  pthread_cond_init(&non_empty,NULL);
  pthread_mutex_init(&mremaining_producers,NULL);

  // get arguments
  char filename[MAX_FILENAME_LENGTH];
  FILE *file;
  strcpy(filename, argv[1]);

  int num_producers = atoi(argv[2]);
  remaining_producers = atoi(argv[2]);
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
  }

  // Close the file
    fclose(file);

  // Now operations array contains the operations read from the file

  // Distribute operations equally among producer threads
  int operations_per_thread = num_operations / num_producers;
  int remaining_operations = num_operations % num_producers;
  int start_index = 0;

  pthread_t producers[num_producers];
  pthread_t consumers[num_consumers];

  for (int i = 0; i < num_consumers ; i++ ) {

    // The consumer threads only need the queue pointer as input
    // Launch consumer threads
    if (pthread_create(&consumers[i], NULL, consumer, (void *)q) != 0) {
     fprintf(stderr, "Error: Unable to create consumer thread\n");
      return 1;
    }
  }

  for (int i = 0; i < num_producers; i++) {
    struct producer_args* thread_args = (struct producer_args*)malloc(sizeof(struct producer_args));
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

  // Wait for consumer threads to finish
  for (int i = 0; i < num_consumers; i++) {
    // Receive the result of the consumer thread (consumer profit and partial stock)
    void *consumer_result;
    if (pthread_join(consumers[i], &consumer_result) != 0) {
      fprintf(stderr, "Error: Unable to join consumer thread\n");
      return 1;
    }
    // Cast the pointer we got from pthread_join to a struct consumer pointer
    struct consumer_result *result_pointer = (struct consumer_result *)consumer_result;
    // Modify the total profit and total stock based on the consumer profit and partial stock
    profits += result_pointer->profit;
    for (int j = 0; j < 5; j++){
      product_stock[j] += result_pointer->partial_stock[j];
    }
    free(result_pointer); // Free the memory allocated for the consumer result struct
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

  // Destroy mutex and variables
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&non_full);
  pthread_cond_destroy(&non_empty);
  pthread_mutex_destroy(&mremaining_producers);

    // Destroy the queue
  queue_destroy(q);

  return 0;
}
