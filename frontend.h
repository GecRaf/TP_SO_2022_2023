#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>




typedef struct item {
    int id;
    char nome[20];
    char categoria[20];
    double valor_base;
    double compre_ja;
}Item;

typedef struct user{

}User;