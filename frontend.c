#include "frontend.h"


void ler_comando(){

    char comando[30];
    printf("funcao le comando");
   /*while(1)*/
}


int main(int argc, char **argv){

    char fun[20];
    printf("Deseja testar que funcionalidade? "); 
    scanf("%s", &fun);
    printf("\n");

    if(fun == "comandos")
    {
        ler_comando();
    }
    //puts(fun);

    

}
