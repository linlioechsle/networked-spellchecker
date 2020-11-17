#include "networked_spellchecker.h"

// create queue data structure
struct Node {
	int client_socket;
	char* word;
	struct sockaddr_in client;
	struct node *next;
};

// declare global variables
char** words; // stores all dictionary words
char* dictionary; // dictionary name
int port_num = 0; // port number

queue <Node> jobQueue;
queue <Node> logQueue;

// declare mutex locks and condition variables

// program functions
char** read_dictionary(char* dict) {
	FILE *file;
	// allocate memory for dictionary
	char** words = (char**)malloc(DICTIONARY_LENGTH*sizeof(char*) + 1);

	// error check memory allocation
	if (words == NULL) {
		printf("%s\n", "error: could not allocate memory for dictionary");
		exit(1);
	}

	// error check dictionary file
	if (!(file = fopen(dict, "r"))) {
		printf("%s\n", "error: could not open dictionary file");
		exit(1);
	}

	file = fopen(dict, "r");
	char* word;

	size_t len = 0;
	ssize_t read = -1;
	int index = 0;

	while ((read = getline(&word, &len, file)) != -1) {
		// allocate enough memory for the size of the word
		words[index] = (char*)malloc(strlen(word)*sizeof(char*) + 1);

	        // error check memory allocation
        	if (word == NULL) {
                	printf("%s\n", "error: could not allocate memory for dictionary word");
                	exit(1);
        	}
		
		strcpy(words[index], word);
		index++;
	}
	fclose(file);
	return words;
}

void *workerThread(void *arg) {
	printf("im in the worker thread");
	return arg;
}

void *logThread(void *arg) {
	return arg;
}

int main(int argc, char* argv[]) {
	// array of worker threads
	pthread_t threads[NUM_THREADS];

	// initialize port and dictionary
	if (argc == 1) { // use default dictionary
		port_num = DEFAULT_PORT;
		dictionary = (char*)DEFAULT_DICTIONARY;
printf("dictionary name: %s\n", dictionary);
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
		if (pthread_create(&threads[i], NULL, &workerThread, NULL) != 0) {
			printf("%s", "error: failed to create worker thread\n");
			exit(1);
		}
	}

	// create logging thread
	// logThread() is the function that the log thread will live in (start routine)
	pthread_t log;
	pthread_create(&log, NULL, &logThread, NULL);

//	while (1) {
//	}
}
