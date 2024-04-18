//SSOO-P3 23/24

#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
  int product_id; //Product identifier
  int op;         //Operation
  int units;      //Product units
};

typedef struct queue {
  // Define the struct yourself
  struct element *buffer; // Array to hold elements
  int head; // position of the head element of the queue
  int tail; // position where the next element will be inserted (enqueued) into the queue
  int max_size; // Maximum max_size of the queue
  //int n_elements; // current size
}queue;

queue* queue_init (int max_size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif
