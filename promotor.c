#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

// Implement function that runs file text in order to execute promotion
// Runs in a thread so that multiple promotions can be ran at the same time

int main(int argc, char **argv){
    char fun[20];
    printf("Deseja testar que funcionalidade? "); 
    scanf("%s", fun);
    printf("\n");
    //puts(fun);
}