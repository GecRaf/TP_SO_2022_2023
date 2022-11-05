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

typedef struct{
    int id;
    char name[20];
    char category[20];
    int basePrice; // Perguntar se é double, int ou float
    int buyNowPrice;
    int duration;
    char sellingUser[20];
    char highestBidder[20]; //User do ultimo que fez proposta mais alta
}Item;