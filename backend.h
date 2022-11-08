#ifndef BACKHEND_H
#define BACKHEND_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include <fcntl.h>
#include "frontend.h"
#define BACKEND_FIFO "/tmp/backend_fifo"


typedef struct user{
    char username[20];
    char password[20];
    int balance;
    pthread_mutex_t mutex;
}User;

typedef struct promotor{
    char category[20];
    int discount;
    int duration;
}Promotor;


#endif // !BACKHEND_H
