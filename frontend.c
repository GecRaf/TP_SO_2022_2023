#include "frontend.h"


void commands(){

    char comando[30];
    printf("funcao le comando");
   /*while(1)*/
}


int main(int argc, char **argv){

    char fun[20];
    printf("Deseja testar que funcionalidade? ");
    scanf("%s", fun);
    printf("\n");

    if(strcmp(fun, "comandos") == 0)
    {
        commands();
    }
    //puts(fun);
}
