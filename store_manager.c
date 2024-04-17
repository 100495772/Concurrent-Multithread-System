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



struct Operation {
  int product_id;
  char op[9]; // PURCHASE or SALE (maximum length 8)
  int units;
};

// Structure used to pass parameters to the threads
struct ThreadArgs {
    int start_index; // Start index of operations for this thread
    int end_index;   // End index of operations for this thread
};

// Producer
void* producer(void* args) {
  struct ThreadArgs* thread_args = (struct ThreadArgs*)args;

  // Process operations assigned to this thread
  for (int i = thread_args->start_index; i < thread_args->end_index; i++) {
      // Process operation operations[i]
      printf("Producer thread processing operation %d\n", i);
  }

  // Free memory allocated for thread arguments
  free(thread_args);

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
  const char* filename = argv[1];
  int num_producers = atoi(argv[2]);
  int num_consumers = atoi(argv[3]);
  int buffer_size = atoi(argv[4]);

  // Open the file for reading
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
      fprintf(stderr, "Error: Unable to open file %s\n", filename);
      return 1;
  }

  // Read the number of operations
  int num_operations;
  char num_operations_str[10]; // Assuming the number of operations will not exceed 9 digits
  int i = 0;
  char ch;
  while (read(fd, &ch, sizeof(char)) == sizeof(char) && ch != '\n') {
      num_operations_str[i++] = ch;
  }
  num_operations_str[i] = '\0';
  num_operations = atoi(num_operations_str);
    
  printf("number of operations is: %d\n", num_operations);

  // Allocate memory for operations array
  struct Operation* operations = (struct Operation*)malloc(num_operations * sizeof(struct Operation));
  if (operations == NULL) {
      fprintf(stderr, "Error: Memory allocation failed\n");
      close(fd);
      return 1;
  }
  
  // Read operations from file
  for (int i = 0; i < num_operations; i++) {
    if (read(fd, &(operations[i].product_id), sizeof(char)) != sizeof(char) ||
        read(fd, &(operations[i].op), sizeof(char) * 8) != sizeof(char) * 8 ||
        read(fd, &(operations[i].units), sizeof(char)) != sizeof(char)) {
        fprintf(stderr, "Error: Unable to read operation from file\n");
        close(fd);
        free(operations);
        return 1;
    }
    // Null-terminate the operation string
    operations[i].op[8] = '\0';
  }
    
  close(fd);

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

    thread_args->start_index = start_index;

    /*We assign values to start and end indexes of each thread
    If i (divisor which represents number of thread) is smaller than remaining operations
    that do not have an assigned thread yet (remainder) it means that we still have
    threads left to create (we need to add 1 to continue with the following thread's arguments)*/
    if (i < remaining_operations) {
      thread_args->end_index = start_index + operations_per_thread + 1;
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

  free(operations);

  return 0;
}
