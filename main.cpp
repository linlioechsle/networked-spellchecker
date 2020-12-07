#include "networked_spellchecker.h"

// create queue data structure
struct Node {
	int client_socket;
	char* word;
	struct sockaddr_in client;
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

int getPort() {
	int port;
	printf("Enter a port number between 1024 and 65535. Enter '1' if you would like to use the default (%d): ", (int)DEFAULT_PORT);
	scanf("%d", &port);
	return port;
}

// returns 0 if word is not found in dictionary, returns 1 if word is found in dictionary
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
int open_listenfd(int port) {
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
	// worker prompt messages
	const char* prompt = "Enter the word that you would like to spellcheck: ";
	const char* closedConnection = "Connection with spellchecker has been closed.\n";
	const char* errorReceiving = "There was an issue with receiving your message.\n";
	// lock job queue
	while (1) {
		pthread_mutex_lock(&lock_jobQueue);
		// job thread waits if queue is empty
		if (jobQueue.size() == 0) {
			pthread_cond_wait(&jobQueueNotEmpty, &lock_jobQueue);
		}

		// dequeue job from jobQueue
		Node job = jobQueue.front();
		jobQueue.pop();

		// release lock, signal that there's an empty spot in the queue
		pthread_mutex_unlock(&lock_jobQueue);
		pthread_cond_signal(&jobQueueNotFull);

		// get client from socket
		int client = job.client_socket;

		// service the client
		while (1) {
			int bytesReturned;
			char word[256] = "";
			// read word from socket
			send(client, prompt, strlen(prompt), 0);
			bytesReturned = recv(client, word, CAPACITY, 0);

			// error check received message
			if (bytesReturned < 0) {
				send(client, errorReceiving, strlen(errorReceiving), 0);
				continue;
			}

			// client disconnected (pressed escape key)
			if (word[0] == 27) {
				send(client, closedConnection, strlen(closedConnection), 0);
				printf("Client has disconnected.\n");
				close(job.client_socket);
				break;
			} else { // check if word is spelled correctly
				char result[100] = "";

				// remove two extra chars counted
				word[strlen(word)-1] = '\0';				
				word[strlen(word)-1] = '\0';

				// add word to result string
				strcat(result, word);

				// add newline char to end of client input
				word[strlen(word)] = '\n';

				int check = spelledCorrectly(word);				
				if (check == 0) { // mispelled
					strcat(result, " MISSPELLED\n");
				} else if (check == 1) { // spelled correctly
					strcat(result, " OK\n");
				}

				// send result to client
				send(client, result, strlen(result), 0);

				// lock log buffer
				pthread_mutex_lock(&lock_logQueue);

				// if log queue is full, wait until there is available space
				if (logQueue.size() > CAPACITY) {
					pthread_cond_wait(&logQueueNotFull, &lock_logQueue);
				}

				// add word and socket response ("OK" or "MISSPELLED") to log queue
				job.word = strcpy(job.word, result);
				printf("WORD TO BE PUSHED TO LOG: %s", job.word);
				logQueue.push(job);

				// signal that log queue is not empty
				pthread_cond_signal(&logQueueNotEmpty);
				// release lock
				pthread_mutex_unlock(&lock_logQueue);
				
			}	
		}
	}
	return arg;
}

void *logThread(void *arg) {
	while (1) {
		// lock job queue
		pthread_mutex_lock(&lock_logQueue);
		// log thread waits if queue is empty
		while(logQueue.size() == 0) {
			pthread_cond_wait(&logQueueNotEmpty, &lock_logQueue);
		}

		// dequeue element from log
		Node entry = logQueue.front();
		logQueue.pop();

		char* word = entry.word;


		// write entry and results to log file
		if (!(log = fopen(DEFAULT_LOG, "a"))) {
			printf("%s\n", "error: could not open log file");
			exit(1);		
		}
		log = fopen(DEFAULT_LOG, "a"); // open log file and allow appending
		fprintf(log, "%s", word);
		fclose(log);

		// signal sleeping threads on the log condition
		pthread_cond_signal(&logQueueNotFull);
		// release lock
		pthread_mutex_unlock(&lock_logQueue);
	}
	return arg;
}

int main(int argc, char* argv[]) {
	// array of worker threads
	pthread_t threads[NUM_THREADS];

	int userPort = getPort();
	if (userPort == 1) {
		portNum = DEFAULT_PORT;
	} else {
		portNum = userPort;
		// error check port number
		if (portNum < 1024 || portNum > 65535) {
			printf("%s\n", "Please enter a port number between 1024 and 65535.");
			exit(1);
		}
	}

	// initialize port and dictionary
	if (argc == 1) { // use default dictionary
		dictionary = (char*)DEFAULT_DICTIONARY;
	} else if (argc == 2) { // use specified dictionary file
			dictionary = argv[1];
	} else { // use specified dictionary file and specified port number
		dictionary = argv[1];
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
		// accept new socket connection
		puts("waiting for new connection");
		// error check connection
		if ((clientSocket = accept(connectionSocket, (struct sockaddr*)&client, &clientLen)) == -1) {
			printf("%s\n", "Error: could not connect to client");
			break;
		}		

		// client messages
		const char* connected = "Connected to server. Enter a word to begin spellchecking.\n";
		const char* fullBuffer = "Job buffer is currently full.\n";

		// lock job queue
		pthread_mutex_lock(&lock_jobQueue);
		// check if job queue is full
		// if full, queue is locked until jobs are processed and queue is no longer full
		if(jobQueue.size() >= CAPACITY) {
			send(clientSocket, fullBuffer, strlen(fullBuffer), 0); // send buffer full message to client
			pthread_cond_wait(&jobQueueNotFull, &lock_jobQueue); // thread sleeps until queue is not full
		}

		puts("Connection successful! Client connected.");

		// if job queue is not full
		send(clientSocket, connected, strlen(connected), 0);

		// enqueue clientSocket to jobQueue
		// create new node, push onto jobQueue
		Node newJob;
		newJob.client_socket = clientSocket;
		newJob.word = (char*)malloc(100*(sizeof(char*)));
		newJob.client = client;

		jobQueue.push(newJob);

		// send condition signal to wake sleping worker threads
		pthread_mutex_unlock(&lock_jobQueue);
		pthread_cond_signal(&jobQueueNotEmpty);

	}


}
