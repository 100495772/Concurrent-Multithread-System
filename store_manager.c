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
pthread_mutex_t mend;   
int end=0; 


int purchase_cost[] = {2, 5, 15, 25, 100};
int selling_price[] = {3, 10, 20, 40, 125};

struct Transaction {
    int product_id;
    const char operation[MAX_OPERATION_LENGTH]; // Operation type: PURCHASE or SALE
    int units;
};

// Structure used to pass parameters to the consumer threads
struct consumer_args {
    queue *q;        // Pointer to the queue
    int *profits;    // Pointer to profits variable
    int *product_stock; // Pointer to prod uct stock array
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


// Consumer
void* consumer(void * args) {
    struct consumer_args *cons_args = (struct consumer_args *)args;
    queue *q = cons_args->q;
    int *profits = cons_args->profits;
    int *product_stock = cons_args->product_stock;

    // Quitar sleep luego
    //sleep(2);
    printf("I'm a consumer\n");

    for (;;) {
      pthread_mutex_lock(&mutex);
      while (queue_empty(q)) {
        if (end==1) {
          fprintf(stderr,"Finishing service\n");
          pthread_mutex_unlock(&mutex);
          pthread_exit(0);
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
          *profits -= units * purchase_cost[product_id - 1];
          product_stock[product_id - 1] += units;
      } else if (operation == 1) { // Sale operation
          *profits += units * selling_price[product_id - 1];
          product_stock[product_id -1 ] -= units;
      }
      //printf("Current Profits: %d\n", *profits);
      //printf("Current Stock for Product %d: %d\n", product_id, product_stock[product_id - 1]);

      // Free the memory for the dequeued element (the memory allocation is made in the queue_get())
      free(element);
    }
    pthread_exit(NULL);
}


// Producer
void* producer(void* args) {

  struct producer_args* pr_args = (struct producer_args*)args;
  // Access thread arguments
  int id = pr_args->id;
  struct Transaction *operations = pr_args->operations;
  int buffer_size = pr_args->buffer_size;
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

    printf("Producer thread %d adding element with Product ID: %d, Operation: %d, Units: %d\n", 
      pr_args->id, elem.product_id, elem.op, elem.units);   

    // Insert element in queue
    queue_put(q, &elem);
    
    // Print the contents of the queue
    printf("Queue Contents:\n");
    for (int i = q->head; i != q->tail; i = (i + 1) % q->max_size) {
      if (queue_empty(q)) {
        printf("Queue is empty\n");
    }
    else {
        printf("Product ID: %d, Operation: %d, Units: %d\n", 
                q->buffer[i].product_id, q->buffer[i].op, q->buffer[i].units);}
    }

    pthread_cond_signal(&non_empty);
    pthread_mutex_unlock(&mutex);


  }

  fprintf(stderr,"Finishing producer\n");
  pthread_mutex_lock(&mend);
  end=1;
  pthread_mutex_unlock(&mend);
  pthread_mutex_lock(&mutex);
  pthread_cond_broadcast(&non_empty);
  pthread_mutex_unlock(&mutex);

  // Free memory allocated for thread arguments 
  free(pr_args);

  fprintf(stderr, "Finished producer\n");

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
  pthread_mutex_init(&mend,NULL);

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
  pthread_t consumers[num_consumers];

  for (int i = 0; i < num_consumers; i++ ) {

    // Create consumer thread arguments
    struct consumer_args c_args = {q, &profits, product_stock};
  
    // Launch consumer threads
    if (pthread_create(&consumers[i], NULL, consumer, (void *)&c_args) != 0) {
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
      fprintf(stderr, "Error: Unable to ioin producer thread\n");
      free(operations);
      return 1;
    }
  }
  
  // Wait for consumer threads to finish
  for (int i = 0; i < num_consumers; i++) {
    if (pthread_join(consumers[i], NULL) != 0) {
      fprintf(stderr, "Error: Unable to ioin consumer thread\n");
      return 1;
    }
  }









    







  // Output
  printf("Total: %d euros\n", profits);
  printf("Stock:\n");
  printf("  Product 1: %d units\n", product_stock[0]);
  printf("  Product 2: %d units\n", product_stock[1]);
  printf("  Product 3: %d units\n", product_stock[2]);
  printf("  Product 4: %d units\n", product_stock[3]);
  printf("  Product 5: %d units\n", product_stock[4]);

  // Free allocated memory
  free(operations);

  // Destroy mutex and variables
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&non_full);
  pthread_cond_destroy(&non_empty);
  pthread_mutex_destroy(&mend);

    // Destroy the queue
  queue_destroy(q);

  return 0;
}
