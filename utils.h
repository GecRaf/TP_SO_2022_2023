#ifndef UTILS_H
#define UTILS_H

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

#define BACKEND_FIFO "backend_fifo"
#define BACKEND_FIFO_FRONTEND "backend_fifo_frontend"
#define FRONTEND_FIFO "frontend_fifo[%d]"
char FRONTEND_FINAL_FIFO[50];

typedef struct backend{
    int connectedClients;
}Backend;

typedef struct user{
    char username[20];
    char password[20];
    int balance;
    int PID;
}User;

typedef struct item{
    int id;
    char name[20];
    char category[20];
    int basePrice; 
    int buyNowPrice;
    int duration;
    char sellingUser[20];
    char highestBidder[20]; 
}Item;

typedef struct promotor{
    char category[20];
    int discount;
    int duration;
}Promotor;

#endif // !BACKHEND_H
