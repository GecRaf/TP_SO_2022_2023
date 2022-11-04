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

typedef struct item {
    int id;
    char name[20];
    char category[20];
    double base_price;
    double buy_now_price;
}Item;

typedef struct user{
    char username[20];
    char password[20];
}User;