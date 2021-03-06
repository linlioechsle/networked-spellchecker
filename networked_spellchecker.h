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
#include <iostream>
#include <cstdlib>
#include <queue>
#include <cstdio>
using namespace std;

// defaults
#define BUF_LEN 512
#define DEFAULT_DICTIONARY "dictionary.txt"
#define DICTIONARY_LENGTH 99171
#define DEFAULT_LOG "log.txt"
#define DEFAULT_PORT 8088
#define NUM_THREADS 10
#define CAPACITY 512

// function declarations
int open_listenfd(int);

#endif
