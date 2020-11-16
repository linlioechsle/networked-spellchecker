#include "networked_spellchecker.h"

// create queue data structure
struct node {
	int client_socket;
	char* word;
	struct sockaddr_in client;
	struct node *next;
};
typedef struct node node;

struct queue {
	int size;
	node* front;
	node* end;
	int capacity;
};
typedef struct queue queue;

// QUEUE FUNCTIONS
// create a new queue and initialize its values
queue* initQueue() {
	queue* tmp = (queue*) malloc(sizeof(queue));
	// error check queue memory allocation
	if (tmp == NULL) {
		printf("%s\n", "error: could not allocate memory for queue");
		exit(1);
	}

	// initialize queue values
	tmp->size = 0;
	tmp->front = NULL;
	tmp->end = NULL;
	tmp->capacity = CAPACITY;
	return tmp;
}

// create a new node and add it to a queue
void enqueue(queue* queue, int client_socket, char* word, struct sockaddr_in client) {
	// first, create and initalize a new node
	// allocate memory for node
	node* entry = (node*) malloc(sizeof(node));
	// error check node memory allocation
	if (entry == NULL) {
		printf("%s\n", "error: could not allocate memory for node");
		exit(1);
	}
	entry->client_socket = client_socket;
	entry->word = malloc(sizeof(char*)*strlen(word) + 1);
	// error check word memory allocation
	if (entry->word == NULL) {
		printf("%s\n", "error: could not allocate memory for word");
		exit(1);
	}
	strcpy(entry->word, word);
	entry->client = client;
	entry->next = NULL;

	// add newly created node to the queue
	if (queue->size == 0) { // queue is empty
		queue->front = entry; // first entry in queue
	} else { // adds new queue entries to end of queue
		node* current = queue->front; 

		while(current->next != NULL) {
			current = current->next;
		}
		current->next = entry;
	}
	// increment size of queue by 1
	queue->size++;
}

// removes first node from queue
node *dequeue(queue* queue) {
	node* tmp = queue->front;

	if (tmp == NULL) { // queue is empty
		return NULL;
	} else { // second node in queue is now first node, prev first node is removed and returned
		queue->front = queue->front->next;
		queue->size--; // decrement queue size
		free(queue->front); // free old first node	
	}
	return tmp;
}



// declare global variables
char** words; // stores all dictionary words
char* dictionary; // dictionary name
int port_num = 0; // port number

queue jobQueue;
queue logQueue;

// declare mutex locks and condition variables
pthread_mutex_t

// program functions
char** read_dictionary(char* file) {

	return
}

void *workerThread() {
	printf("im in the worker thread");
}

void *logThread() {

}

int main(int argc, char* argv[]) {
	// array of worker threads
	pthread_t threads[NUM_THREADS];


	// initialize port and dictionary
	if (argc == 1) { // use default dictionary
		port_num = DEFAULT_PORT;
		dictionary = DEFAULT_DICTIONARY;
	} else if (argc == 2) { // use specified dictionary file
		port_num = DEFAULT_PORT;
		dictionary = argv[1];
	} else { // use specified port and specified dictionary file
		port_num = atoi(argv[1]);

		// error check port number
		if (port_num < 1024 || port_num > 65535) {
			printf("%s\n", "Please enter a port number between 1024 and 65535.");
			exit(1);
		}

		dictionary = argv[2];
	}

	// read dictionary from file
	words = read_dictionary(dictionary);

	// create NUM_THREADS number of worker threads
	// workerThread() is the function that the threads will live in (start routine)
	// create worker threads
	for (size_t i = 0; i < NUM_THREADS; ++i) {
		if (pthread_create(&threads[i], NULL, workerThread, NULL) != 0) {
			printf("%s", "error: failed to create worker thread\n");
			exit(1);
		}
		pthread_create(&threads[i], NULL, workerThread, NULL);
	}

	// create logging thread
	// logThread() is the function that the log thread will live in (start routine)
	pthread_create(&logThr, NULL, logThread, NULL);

//	while (1) {
//	}
}
