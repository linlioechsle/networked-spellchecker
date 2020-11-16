#ifndef _NETWORKED_SPELLCHECKER_H
#define _NETWORKED_SPELLCHECKER_H
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

// defaults
#define BUF_LEN 512
#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_LOG "log.txt"
#define DEFAULT_PORT 1723
#define NUM_THREADS 5
#define CAPACITY 500
// function declarations
int open_listenfd(int);

#endif
