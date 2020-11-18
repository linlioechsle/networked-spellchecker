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
int portNum = 0; // port number
FILE* log;

queue <Node> jobQueue;
queue <Node> logQueue;

// declare mutex locks and condition variables
pthread_mutex_t lock_jobQueue;
pthread_mutex_t lock_logQueue;
pthread_cond_t jobQueueNotFull;
pthread_cond_t jobQueueNotEmpty;
pthread_cond_t logQueueNotFull;
pthread_cond_t logQueueNotEmpty;

// program functions
// reads dictionary file and stores words in char**
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
	words[index] = NULL;
	fclose(file);
	return words;
}

// returns 0 if word is not found in dictionary, returns 1 if word is found in dictionary
// CURRENT ENTRIES MUST HAVE \n IN ORDER TO BE FOUND AS CORRECTLY SPELLED. THIS MUST BE HANDLED.
int spelledCorrectly (char* input) {
	// 0 = mispelled, 1 = spelled correctly
	int index = 0;
	while(words[index] != NULL) {
		if (strcmp(words[index], input) == 0) { // word found
			return 1;
		}
		index++;
	}
	return 0;
}

//Copied from the Computer Systems textbook.
//This function creates a socket descriptor, and binds
//the socket descriptor the specified port.
//bind() associates the socket descriptor created
//with socket() to the port we want the server to listen on.
//Once the descriptor is bound, the listen() call
//will prepare the socket so that we can call accept() on it
//and get a connection to a user.
int open_listenfd(int port)
{
        int listenfd, optval=1;
        struct sockaddr_in serveraddr;

        /* Create a socket descriptor */
        if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                return -1;
        }

         /* Eliminates "Address already in use" error from bind */
         if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
         (const void *)&optval , sizeof(int)) < 0){
                return -1;
         }

         //Reset the serveraddr struct, setting all of it's bytes to zero.
         //Some properties are then set for the struct, you don't
         //need to worry about these.
         //bind() is then called, associating the port number with the
         //socket descriptor.
         bzero((char *) &serveraddr, sizeof(serveraddr));
         serveraddr.sin_family = AF_INET;
         serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
         serveraddr.sin_port = htons((unsigned short)port);
         if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
                return -1;
         }

         //Prepare the socket to allow accept() calls. The value 20 is
         //the backlog, this is the maximum number of connections that will be placed
         //on queue until accept() is called again.
         if (listen(listenfd, 20) < 0){
                return -1;
         }

         return listenfd;
}

void *workerThread(void *arg) {
	while (1) {
	// lock job queue
	pthread_mutex_lock(&lock_jobQueue);
	// job thread waits if queue is empty
	if (jobQueue.size == 0) {
		pthread_cond_wait(&jobQueueNotEmpty, &lock_jobQueue);
	}

	// dequeue job from jobQueue
	Node *job = jobQueue.pop();
	// release lock, signal that job queue is not full
	pthread_mutex_unlock(&lock_jobQueue);
	pthread_cond_signal(&jobQueueNotFull);

	// get client from socket
	int client = job.client_socket;
printf("client after popping from jobQueue: %d\n", client);

	// read word from socket
	// check if input indicated EOI
	// close socket
	// close(job->socket);
	// check if word is in dictionary
	// add message to log queue and return message to client
	// if word is in dictionary
		// word OK
	// else
		// word MISPELLED

	}
	return arg;
}

void *logThread(void *arg) {
	while (1) {
		// lock job queue
		pthread_mutex_lock(&lock_logQueue);
		// log thread waits if queue is empty
		while(logQueue.size == 0) {
			pthread_cond_wait(&logQueueNotEmpty, &lock_logQueue);
		}

		// dequeue element from log
		Node* entry = logQueue.pop();
		char* word = entry.word;


		// write entry and results to log file
		if (!(log = fopen(DEFAULT_LOG, "a"))) {
			printf("%s\n", "error: could not open log file");
			exit(1);		
		}
		log = fopen(DEFAULT_LOG, "a"); // open log file and allow appending
		fprintf(log, "%s", word);
		fclose(log);
// handle also printing out the result of mispelled or correctly spelled on the same line
		// signal sleeping threads on the log condition
		pthread_cond_signal(&logQueueNotFull);
		// release the lock
		pthread_mutex_unlock(&lock_logQueue);
	}
	return arg;
}

int main(int argc, char* argv[]) {
	// array of worker threads
	pthread_t threads[NUM_THREADS];

	// initialize port and dictionary
	if (argc == 1) { // use default dictionary
		portNum = DEFAULT_PORT;
		dictionary = (char*)DEFAULT_DICTIONARY;
	} else if (argc == 2) { // use specified dictionary file
		portNum = DEFAULT_PORT;
		dictionary = argv[1];
	} else { // use specified port and specified dictionary file
		portNum = atoi(argv[1]);

		// error check port number
		if (portNum < 1024 || portNum > 65535) {
			printf("%s\n", "Please enter a port number between 1024 and 65535.");
			exit(1);
		}

		dictionary = argv[2];
	}
printf("dictionary name: %s\n", dictionary);

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
char* testing = (char*)"cat\n";
printf("SPELLED CORRECTLY: %d\n", spelledCorrectly(testing));
	// create logging thread
	// logThread() is the function that the log thread will live in (start routine)
	pthread_t log;
	pthread_create(&log, NULL, &logThread, NULL);

	// create socket and accept client connections
	struct sockaddr_in client;
	socklen_t clientLen = sizeof(client);
	int clientSocket;
	// listens for connection
	int connectionSocket = open_listenfd(portNum);

	if(connectionSocket == -1) {
		printf("%s\n", "error: could not connect to port");
		exit(1);
	}

	while (1) {
		puts("waiting for connection");
		// accept new socket connection
		// error check connection
		if ((clientSocket = accept(connectionSocket, (struct sockaddr*)&client, &clientLen)) == -1) {
			printf("%s\n", "error: could not connect to client");
			break;
		}		
		puts("Connection successful! Client connected.");

		// client messages
//		char* connected = "Connected to server. Enter a word to begin spellchecking.";
//		char* fullBuffer = "Job buffer is currently full.";

		// lock job queue
		pthread_mutex_lock(&lock_jobQueue);
		// check if job queue is full
		// if full, queue is locked until jobs are processed and queue is no longer full
/*		if(jobQueue.size > CAPACITY) {
			send(clientSocket, fullBuffer, strlen(fullBuffer), 0); // send buffer full message to client
			pthread_cond_wait(&jobQueueNotFull, &lock_jobQueue); // thread sleeps until queue is not full
		}
*/
		// if job queue is not full
//		send(clientSocket, connected, strlen(connected), 0);

		// enqueue clientSocket to jobQueue
		// create new node, push onto jobQueue
//		jobQueue.push(

		// send condition signal to wake sleping worker threads
		pthread_mutex_unlock(&lock_jobQueue);


	}


}
