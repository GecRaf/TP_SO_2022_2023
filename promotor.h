#ifndef PROMOTOR_H
#define PROMOTOR_H

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


//enviar ao backend 
typedef struct promotor{
    char category[20];
    int discount;
    int duration;
}Promotor;

#endif // !PROMOTOR_H
