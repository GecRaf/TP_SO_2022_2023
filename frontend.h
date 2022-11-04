#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

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